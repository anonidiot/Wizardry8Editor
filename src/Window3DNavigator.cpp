/*
 * Copyright (C) 2022-2024 Anonymous Idiot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Significant portions of this source code are based on the Urho3d Samples,
 * most significantly the CharacterDemo sample. Copyright from the sample follows:
 *
 * Copyright (c) 2008-2022 the Urho3D project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Window3DNavigator.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

#include <Urho3D/IO/Log.h>

#include "Avatar.h"
#include "Touch.h"
#include "Level.h"
#include "UrhoTools.h"

#include "SLFFile.h"
#include "SLFDeserializer.h"

#if defined(WIN64)
#include <Urho3D/ThirdParty/SDL/SDL_syswm.h>

typedef UINT (WINAPI *farprocGetDpiForWindow)(HWND hwnd);
#endif

static const float kAvatarOffset          =  960.0f;   /** vertical dist differential between Avatar y-coord and Wizardry y-coord */
static const float kScale                 =  0.0013f;
static const float kLightIntensityScale   =  0.2f;
static const float kCameraInitialDistance =  5.0f;
static const float kCameraMinDistance     =  1.0f;
static const float kCameraMaxDistance     = 20.0f;

static const float kEpsilon               = 1e-9;
static const float kSloppyEpsilon         = 1e-3;

static const float TOUCH_SENSITIVITY      = 2.0f;

static const int   VERTICES_PER_FACE      = 3;

using namespace Urho3D;


typedef struct
{
    float  x;
    float  y;
    float  z;
    float  nx;
    float  ny;
    float  nz;
    float  u;
    float  v;
} vertex_t;

typedef struct
{
    int    idx;
    float  u;
    float  v;
} vertref_t;

struct face
{
    vertref_t   vert[VERTICES_PER_FACE];
    int         material_idx;
};

typedef struct
{
    float  u;
    float  v;
} uv_t;

struct animated_mesh
{
    char      object_name[64];
    char      trigger_name[128];

    float     origin_x;
    float     origin_y;
    float     origin_z;

    float     fps;

    bool      expect_matrix;

    uint16_t  num_frames;
    uint32_t  num_vertices;
    uint32_t  num_faces;
    uint32_t  num_materials;
    
    vertex_t    **frames;
    face_t       *faces; 
    uint8_t     **material_buf;
};


static void bitmapDetails(SLFFile *f, bool bBitmapDirSet, struct animated_mesh *ani_ptr);
static void duplicateVerticesForUVs( face_t *faces, int num_faces, vertex_t **vertices, uint32_t *num_vertices, int num_frames );

extern bool  getBoolSetting(char *setting);
extern void  setBoolSetting(char *setting, bool value);


Window3DNavigator::Window3DNavigator( int mapId, float x, float y, float z, float heading, bool isSavePosition, int32_t *visitedMaps )
    : Object(new Urho3D::Context()),
    firstPerson_(false),
    arrivalPts_(NULL),
    maps_(NULL),
    yaw_(0.0f),
    pitch_(0.0f),
    touchEnabled_(false),
    hiDpiScale_(1.0),
    screenJoystickIndex_(M_MAX_UNSIGNED),
    screenJoystickSettingsIndex_(M_MAX_UNSIGNED),
    mapVisible_(false),
    positionInfoVisible_(true),
    noclipActive_(false),
    isSavePosition_(isSavePosition),
    visitedMaps_(visitedMaps), // yes, we're only copying the pointer, don't free it in the caller
    mapId_(-1),                // doesn't get populated till level loaded
    newMap_(false),
    saveChanges_(false)
{
    engine_ = new Urho3D::Engine(context_);

    VariantMap engineParameters;

    engineParameters[ EP_WINDOW_TITLE      ] = "Wizardry Navigator - <ESC> releases/grabs the mouse";
    engineParameters[ EP_FRAME_LIMITER     ] = false;
    engineParameters[ EP_HEADLESS          ] = false;
    engineParameters[ EP_SOUND             ] = false;
    engineParameters[ EP_FULL_SCREEN       ] = false;
    // We force it to load at the top left of the screen because if we're on
    // a highDpi monitor the window will need to resize, and if it puts it in
    // the default location (centred) it ends up going off the lower right corner
    // of the screen. We don't use 0,0 because otherwise Windows hides the
    // titlebar
    engineParameters[ EP_WINDOW_POSITION_X ] = 10;
    engineParameters[ EP_WINDOW_POSITION_Y ] = 60;

    engineParameters[ EP_RESOURCE_PATHS  ] = ":/CoreData.pak";

    if (engine_->Initialize( engineParameters ))
    {
#if defined(WIN64)
        ScaleForHiDPI();
#endif

        context_->RegisterSubsystem(this);
        context_->RegisterSubsystem(engine_);

        // ALT-ENTER can toggle back and forth between window and full screen
        // unless we block it. This isn't working properly in OpenGL on either
        // Linux or Windows though, so we block it unless we're using DirectX.
#ifdef URHO3D_OPENGL
        GetSubsystem<Input>()->SetToggleFullscreen(false);
#endif

        // Doesn't come back true on Windows when using HiDPI, regardless of
        // whether the engine has support for it turned on or not
        // isHiDpi_ = GetSubsystem<Graphics>()->GetHighDPI();

        // including this line hides our toolbar
        Avatar::RegisterObject(context_);

        context_->RegisterFactory<WizardryAnimatedMesh>();

        if (GetSubsystem<Input>()->GetNumJoysticks() == 0)
            // On desktop platform, do not detect touch when we already got a joystick
            SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(Window3DNavigator, HandleTouchBegin));

        // Toggle between window and fullscreen
        SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(Window3DNavigator, HandleScreenModeChange));

        // Subscribe key down event
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Window3DNavigator, HandleKeyDown));
        // Subscribe scene update event
        //SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Window3DNavigator, HandleSceneUpdate));

        if (touchEnabled_)
            touch_ = new Touch(context_, TOUCH_SENSITIVITY);

        // Create static scene content
        CreateScene(mapId);

        // Create toolbar
        CreateUI();

        CreateMapsWindow();
        CreateArrivalPointWindow();

        CreateAvatar();

        // Initialise state without flipping it
        mapVisible_          = getBoolSetting( "navigatorMapOn"    );
        positionInfoVisible_ = getBoolSetting( "navigatorInfoOn"   );
        noclipActive_        = getBoolSetting( "navigatorNoClipOn" );

        ToggleMap(false);
        TogglePositionInfo(false);
        ToggleNoclipMode(false);

        // Instructions area bit intrusive, and ultimately I'd like the controls customisable anyway
      //CreateInstructions();

        ShowMapsWindow(false, false);

        if (!isnan(x) && !isnan(y) && !isnan(z))
        {
            ShowLocationsWindow(false, false);
            SetAvatarPosition(Vector3( x, y, z), heading);
        }
        else
        {
            ShowLocationsWindow(true, true);
        }
    }
}

Window3DNavigator::~Window3DNavigator()
{
    // It's great how Urrho defined a function called Pop() that doesn't actually return anything;
    // it's not like it's a standard name or anything.

    while (! waterPlanes_.Empty() )
    {
        delete waterPlanes_.Back();
        waterPlanes_.Pop();
    }
    while (! materials_.Empty() )
    {
        delete materials_.Back();
        materials_.Pop();
    }
    while (maps_)
    {
        struct map *m = maps_->next;

        free(maps_);
        maps_ = m;
    }
    while (arrivalPts_)
    {
        struct location *a = arrivalPts_->next;

        free(arrivalPts_);
        arrivalPts_ = a;
    }
    while (! stuffToFreeLater_.Empty() )
    {
        free( stuffToFreeLater_.Back() );
        stuffToFreeLater_.Pop();
    }
}

int Window3DNavigator::exec()
{
    while (!engine_->IsExiting())
        engine_->RunFrame();

    setBoolSetting( "navigatorMapOn",    mapVisible_          );
    setBoolSetting( "navigatorInfoOn",   positionInfoVisible_ );
    setBoolSetting( "navigatorNoClipOn", noclipActive_        );

    if (saveChanges_)
        return Window3DNavigator::DialogCode::Accepted;

    if (newMap_)
        return Window3DNavigator::DialogCode::ChangeMap;

    return Window3DNavigator::DialogCode::Rejected;
}

void Window3DNavigator::ScaleForHiDPI()
{
#if defined(WIN64)
    SDL_SysWMinfo sysInfo;
    HWND          hWnd;

    Graphics     *g       = GetSubsystem<Graphics>();
    SDL_Window   *window  = g->GetWindow();

    SDL_VERSION(&sysInfo.version);
    SDL_GetWindowWMInfo(window, &sysInfo);
    hWnd = sysInfo.info.win.window;

    int dpi = 0;
 #if WINVER >= 0x0605
    dpi = GetDpiForWindow(hWnd);
 #else
    // We're compiling for compatibility with older versions of Windows
    // so check if the function is available from the OS at runtime
    farprocGetDpiForWindow pFunction = (farprocGetDpiForWindow) GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetDpiForWindow");
    if (pFunction)
    {
        dpi = pFunction(hWnd);
    }
 #endif
    if (dpi != 0)
    {
        // The 1024 and 768 values are internal defaults used for Engines that don't
        // specify width or height
        int dpiScaledWidth  = MulDiv(1024, dpi, 96);
        int dpiScaledHeight = MulDiv(768, dpi, 96);

        // This can make the Window go off the lower right hand corner of the screen
        g->SetMode( dpiScaledWidth, dpiScaledHeight );

        hiDpiScale_ = (double) dpiScaledWidth / 1024.0;
    }
#endif
}

bool Window3DNavigator::isTGAFile32Bit( String tga_file )
{
    SLFFile tga( "LEVELS", "LEVELS.SLF", tga_file.CString(), false );

    if (tga.isGood())
    {
        tga.open(QFile::ReadOnly);
        tga.skip(16);
        if (tga.readByte() == 32)
            return true;
    }

    return false;
}

void Window3DNavigator::AddTgaToResourceCache( String texture_name )
{
    AddTgaToResourceCache( "Levels", "LEVELS.SLF", texture_name );
}

void Window3DNavigator::AddTgaToResourceCache( String folder, String slf, String texture_name )
{
    auto* cache = GetSubsystem<ResourceCache>();

    SharedPtr<Texture2D> renderTexture = context_->CreateObject<Texture2D>();
    if (renderTexture)
    {
        SLFFile         tga( folder.CString(), slf.CString(), texture_name.CString(), false );

        if (tga.isGood())
        {
            SLFDeserializer tgaDeserial( tga );

            SharedPtr<Resource> resource = DynamicCast<Resource>(renderTexture);
            if (resource)
            {
                resource->SetName( texture_name );
                resource->Load( tgaDeserial );

                cache->AddManualResource( resource );
            }
        }
    }
}

int Window3DNavigator::LoadMaterial( String pvlFolderName, uint8_t *material_buf )
{
    bool    emissive = false;

    String material_name = String((char *)material_buf).ToUpper();
    String texture_file  = String();

    if (*(material_buf+40))
        texture_file = String((char *)material_buf+40).ToUpper();
    else if (*(material_buf+80))
        texture_file = String((char *)material_buf+80).ToUpper();
    else if (*(material_buf+120))
        texture_file = String((char *)material_buf+120).ToUpper();
    else if (*(material_buf+160))
    {
        texture_file = String((char *)material_buf+160).ToUpper();
        emissive = true;
    }

    // FIXME:
    // is_non_collidable is NOT entirely the same as transparent. There are textures, eg Cemetary gate
    // which need to be transparent and are collidable. There doesn't seem to be any flag for
    // transparency - you're going to have to inspect each and every TGA
    // Others like the moss under the Trangporter house should be both transparent and non-collidable
    // (unless you can fix it with a height adjustment of Jack)
    bool has_alpha         = material_buf[0x115]; // not _reliably_ trustworthy though

  //float *ambient      = (float *)(material_buf+0xc8); //[r,g,b]
    float *diffuse      = (float *)(material_buf+0xd4); //[r,g,b]
    float *transmit     = (float *)(material_buf+0xe0); //[r,g,b]
    float *specular     = (float *)(material_buf+0xec); //[r,g,b]

    float alpha     = *(float *)(material_buf+0xfc);
  //float shininess = *(float *)(material_buf+0x100);
    float fps       = *(float *)(material_buf+0x111);

    float  actualColor[4];

    // If diffusion color not set but transmit color is
    if ((fabsf(diffuse[0]) < kEpsilon) &&
        (fabsf(diffuse[1]) < kEpsilon) &&
        (fabsf(diffuse[2]) < kEpsilon) &&
        ((fabsf(transmit[0]) >= kEpsilon) ||
         (fabsf(transmit[1]) >= kEpsilon) ||
         (fabsf(transmit[2]) >= kEpsilon)))
    {
        // I haven't had any success getting MatEmissiveColor to work correctly with any of the shaders
        // so just override diffuse with it
        actualColor[0] = transmit[0];
        actualColor[1] = transmit[1];
        actualColor[2] = transmit[2];
        actualColor[3] = 0.5;
    }
    else
    {
      //renderMaterial->SetShaderParameter( "AmbientColor",     Vector4( ambient[0],  ambient[1],  ambient[2],  alpha ) );
        actualColor[0] = diffuse[0];
        actualColor[1] = diffuse[1];
        actualColor[2] = diffuse[2];
        actualColor[3] = alpha;
    }

    StringHash hash( pvlFolderName + material_name + " " + texture_file + " " + (emissive?"T":"F") + " " + String(alpha) + " " + String(fps) + " " + String(actualColor[0]) + "," + String(actualColor[1]) + "," + String(actualColor[2]) + "," + String(actualColor[3]) + " " + String(specular[0]) + "," + String(specular[1]) + "," + String(specular[2]));
    String hash_s = hash.ToString();

    for (int k=0; k < (int)materials_.Size(); k++)
    {
        WizardryMaterial *m = materials_[k];

        if (m->hash_ == hash_s)
        {
            // We already have a material for this texture - put in an empty record in this slot
            // that maps through to an earlier record already created for it.
            WizardryMaterial  *wm = new WizardryMaterial("");

            wm->mapsTo_ = k;

            materials_.Push( wm );
            return materials_.Size()-1;
        }
    }

    WizardryMaterial  *wm = new WizardryMaterial(hash_s);

    auto* cache = GetSubsystem<ResourceCache>();

    SharedPtr<Material> renderMaterial(new Material(context_));

    renderMaterial->SetShaderParameter( "MatDiffColor",     Vector4( actualColor[0],  actualColor[1],  actualColor[2],  actualColor[3] ) );

    // Enabling this seriously messes up the rendering of untextured surfaces like the statue in Arnika
    // _ANY_ alpha value at all (even 0.0) causes it to show up as bright white
  //renderMaterial->SetShaderParameter( "MatEmissiveColor", Vector4( transmit[0], transmit[1], transmit[2], alpha ) );
    renderMaterial->SetShaderParameter( "MatSpecColor",     Vector4( specular[0], specular[1], specular[2], alpha ) );
  //renderMaterial->SetAlphaToCoverage(true);

    // I can't find anything to help me know when something is solid or not.
    wm->isCollidable_ = true;

    // I think the collision detection in Wizardry is done entirely in the Octree. There's no field in the material
    // or the mesh I can see for storing whether it is passthrough or not. I'm not doing an Octree parser just for
    // this, so we have this ugly hack to just list certain materials as passthrough. Obviously it isn't going to
    // work for custom levels unless they name their materials one of these.
    // (the material file "water.ifl" comes up far more consistently than any texture name for water, so we use
    //  that to keep this check shorted, rather than comparing material names)
    if ((texture_file == "WATER.IFL")            ||
        (texture_file == "WATER-0000.IFL")       ||
        (texture_file == "WATER-0010.IFL")       ||
        (texture_file == "WATERFLOW.IFL")        ||
        (texture_file == "WATERFLOW0019.IFL")    ||
        (texture_file == "MBWATER-001A.IFL")     ||
        (texture_file == "S_WATER-0006.IFL")     ||
        (texture_file == "DCWATER0000.IFL")      ||
        (texture_file == "MUD.IFL")              ||
        (texture_file == "SLIME-0000.IFL")       ||
     // (texture_file == "B1-LIQUID0000.IFL")    || -- not this one because no floor beneath it in some places
        (texture_file == "TR1-WATERFROTH_A.TGA") ||
        (texture_file == "B-WATERFROTH_A.TGA")   ||
        (texture_file == "B-WATER_A.TGA")        ||
        (texture_file == "WATER_A.TGA")          ||
        (texture_file == "WATER.TGA"))
    {
        wm->isCollidable_ = false;

        // Water gets special treatment. It is rendered translucent so that a reflection can be
        // drawn just below it (hard to see because of the brightness of our water, but it's there
        // Rendering it with an alpha transparency isn't enough to get this to work on its own.
        wm->isWater_      = true;
    }
    if (texture_file == "LAVA0000.IFL")
    {
        wm->isCollidable_ = false;
    }

    if ( texture_file.Empty() || (texture_file == "NOTEXTURE"))
    {
        renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/NoTextureAOAlpha.xml"));
    }
    else // has a texture
    {
        if (texture_file.EndsWith(".IFL")) // animation
        {
            String ifl_file = pvlFolderName + "/BITMAPS/" + texture_file;
            SLFFile  ifl( "Levels", "LEVELS.SLF", ifl_file.CString(), false );
            SLFDeserializer iflDeserial( ifl );
            String line;

            while ((line = iflDeserial.ReadLine()) != "")
            {
                wm->textureNames_.Push( pvlFolderName + "/BITMAPS/" + line.ToUpper() );
            }

            wm->textureUpdateRate_ = (1.0 / fps);
        }
        else // static texture
        {
            wm->textureNames_.Push( pvlFolderName + "/BITMAPS/" + texture_file );
        }

        // We add all textures to the cache - animated because they load so frequently, and
        // static because some levels render terrible enough as it is without making it worse.
        for (int k=0; k < (int)wm->textureNames_.Size(); k++)
        {
            AddTgaToResourceCache( wm->textureNames_[k] );
        }

        if ((alpha == 0.0) || wm->isWater_)
        {
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffAlphaTranslucent.xml"));

            if ((texture_file == "VOLUMECLIGHT.TGA") ||       // Large alpha areas at player height
                (texture_file == "BEAM-000A.IFL"))
            {
                wm->isCollidable_ = false;
            }
        }
        else if ((alpha < 1.0) || isTGAFile32Bit( wm->textureNames_[0] ))
        {
            // DiffLitParticleAlpha.xml for fire particle ?
            // DiffAOAlpha is too dark
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffAlpha.xml"));
            // The shadows of the trees don't render correctly with DiffAlpha - they are the
            // shadows of the squares containing them. This can be fixed by copying the technique
            // to a new file, eg. Techniques/DiffAlphaShadows.xml and adding in a psdefines="ALPHAMASK"
            // parameter. We do the same thing here in code to avoid creating a custom technique
            Pass *p = renderMaterial->GetTechnique(0)->GetPass("shadow");
            p->SetPixelShaderDefines("ALPHAMASK");

            // It's not perfect - the shadows update with a definite raster effect as you move
            // past them, as the differing viewpoint on the tree causes alphas to change.
            // At least it's better than the rectangular shadow for a round tree you get without it.

            // Urho3d lacks the ability to make objects semi-collidable (ie. collide if textured
            // in the area of collision but not collide if it is transparent). Possibly something
            // could be implemented via the node collision event, but I can't see it being simple
            // or efficient. Without remeshing the objects ourselves we have to choose between
            // making an object completely collidable or non-collidable.
            // Mostly this isn't too bad; we can make the trunks of the trees solid, and the leaves
            // non-collidable, for instance. But irritatingly some of the trees just use the one
            // texture for both leaves and trunk - so we either have a completely impassable tree
            // (which makes some levels unnavigable) or we have a tree we can walk right through the
            // middle of. Because this is an editor and not a game itself, the lesser evil appears to
            // be letting some trees be walked through.
            // Just don't set portals or save points inside trees please! There's no detection to
            // prevent this but it will mess up the game.
            if ((texture_file == "B-BAMBOOBUSH_A.TGA")    ||  // Large alpha areas at player height
                (texture_file == "B-FANCORAL_A.TGA")      ||
                (texture_file == "B-FANCORALRED_A.TGA")   ||
                (texture_file == "B-FANFERN_A.TGA")       ||
                (texture_file == "B-FANFERNDEAD_A.TGA")   ||
                (texture_file == "B-SEAWEED_A.TGA")       ||
                (texture_file == "B1-COBWEB_A.TGA")       ||
                (texture_file == "B1-GREENSMOKE_A.TGA")   ||
                (texture_file == "B1-MOSS-SINGLE_A.TGA")  ||
                (texture_file == "B1-ROOTS_A.TGA")        ||
                (texture_file == "B1-VINEANDMOSS_A.TGA")  ||
                (texture_file == "B1-WEBPILLAR_A.TGA")    ||
                (texture_file == "B1-WEBS_A.TGA")         ||
                (texture_file == "B2-CLIFWEEDS_A.TGA")    ||
                (texture_file == "BUSH_A.TGA")            ||
                (texture_file == "BUSHES1_A.TGA")         ||
                (texture_file == "CE-MIST_A.TGA")         ||
                (texture_file == "CT-DROOPY_A.TGA")       ||
                (texture_file == "CLIFWEEDS_A.TGA")       ||
                (texture_file == "DROOPY_A.TGA")          ||
                (texture_file == "EGG-WEB_A.TGA")         ||
                (texture_file == "GRASSBLADE_A.TGA")      ||
                (texture_file == "MT-BRANCHTREE_A.TGA")   ||
                (texture_file == "S_BLUEFLOWERS.TGA")     ||
                (texture_file == "S_BLUEFLOWERS2.TGA")    ||
                (texture_file == "S_FLOWERSORANGE_A.TGA") ||
                (texture_file == "S_PATCHWORK_01.TGA")    ||
                (texture_file == "S_TALL_BUSH.TGA")       ||
                (texture_file == "S_TALL_GRASS.TGA")      ||
                (texture_file == "S_TREE_BITMAP2.TGA")    ||
                (texture_file == "S_TREE_LEAVES.TGA")     ||
                (texture_file == "S_WEEPING.TGA")         ||
                (texture_file == "S_WEEPING_2.TGA")       ||
                (texture_file == "S_YELFLOWERS.TGA")      ||
                (texture_file == "SMOKE.TGA")             ||
                (texture_file == "TR-DROOPY_A.TGA")       ||
                (texture_file == "TR1-DEADTREE_A.TGA")    ||
                (texture_file == "TR1-SHRUBBS_A.TGA")     ||
                (texture_file == "TR1-TREEGREEN_A.TGA")   ||
                (texture_file == "TR1-TREEGREEN.TGA")     ||
                (texture_file == "TR1-TREERED_A.TGA")     ||
                (texture_file == "TR1-TREERED.TGA")       ||
                (texture_file == "TR1-WILLOW_A.TGA")      ||
                (texture_file == "TR2_FLOWERS_A.TGA")     ||
                (texture_file == "TR2_GARLIC_A.TGA")      ||
                (texture_file == "TR2_SPIDERPLANT_A.TGA") ||
                (texture_file == "TR2_VINEGROWTH_A.TGA")  ||
                (texture_file == "TR2-VINEANDMOSS_A.TGA") ||
                (texture_file == "UM1_HANGINGSLIME.TGA")  ||
                (texture_file == "UM2-VINEANDMOSS_A.TGA") ||
                (texture_file == "UM2-VINEGROWTH_A.TGA")  ||
                (texture_file == "WEBS_A.TGA")            ||
                (texture_file == "YU_WEEDS_A.TGA"))
            {
                wm->isCollidable_ = false;
            }
        }
        else if (emissive)
        {
            // FIXME:
            // These do not render correctly, even if the emissiveColor is set and disabling the diffuse
            // color. I don't know enough to know how to fix it, but think it needs a custom shader.
            // It's supposed to render so that the black parts are completely transparent and the lighter
            // parts glow. There's no alpha channel in these images.
#if 0
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffEmissive.xml"));
#else
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/Diff.xml"));
#endif
            if ((texture_file == "B1-WEB_TILE.TGA")     ||  // Large alpha areas at player height
                (texture_file == "B1-WEB_TILE_END.TGA") ||
                (texture_file == "B1-WEB_TILE_T.TGA")   ||
                (texture_file == "B1-BEAMORANG.TGA"))
            {
                wm->isCollidable_ = false;
            }
        }
        else
        {
            // DiffAO is too dark
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/Diff.xml"));
        }

        Texture2D *renderTexture = cache->GetResource<Texture2D>( wm->textureNames_[0] );
        renderMaterial->SetTexture(TU_DIFFUSE, renderTexture);
    }


    // Wizardry TGAs don't need UV flipping - Urrho3D wants them the same way;
    // one of these should have done it if it WAS needed though

    //   renderMaterial->SetShaderParameter( "VOffset",  Vector4( 0.0, -1.0, 0.0, 2.0 ) );
    //   renderMaterial->SetShaderParameter( "VOffset",  Vector4( 0.0, -1.0, 0.0, 1.0 ) );
    wm->SetMaterial( renderMaterial );

    materials_.Push( wm );
    return materials_.Size()-1;
}

Vector<int32_t> Window3DNavigator::CollateMaterials(face_t *faces, int num_faces)
{
    Vector<int32_t> collated_materials;

    int last_tex_id = -1;
    for (int k=0; k<num_faces; k++)
    {
        if (last_tex_id != faces[k].material_idx)
        {
            last_tex_id = faces[k].material_idx;

            if (! collated_materials.Contains(last_tex_id))
            {
                collated_materials.Push(last_tex_id);
            }
        }
    }
    return collated_materials;
}

static void duplicateVerticesForUVs( face_t *faces, int num_faces, vertex_t **vertices, uint32_t *num_vertices, int num_frames )
{
    bool *uv_assigned = (bool *)calloc( *num_vertices, sizeof(bool) );

    int orig_num_vertices = *num_vertices;

    for (int k=0; k<num_faces; k++)
    {
        for (int j=0; j<VERTICES_PER_FACE; j++)
        {
            vertref_t *f_ptr = &(faces[k].vert[j]);

            if (! uv_assigned[ f_ptr->idx ])
            {
                // unassigned vertex - take it
                for (int j=0; j<num_frames; j++)
                {
                    vertices[ j ][ f_ptr->idx ].u = f_ptr->u;
                    vertices[ j ][ f_ptr->idx ].v = f_ptr->v;
                }

                uv_assigned[ f_ptr->idx ] = true;
            }
            else
            {
                // Check if the entry that is already in there matches our
                // desired UV
                if ((fabsf(vertices[ 0 ][ f_ptr->idx ].u - f_ptr->u) < kEpsilon) &&
                    (fabsf(vertices[ 0 ][ f_ptr->idx ].v - f_ptr->v) < kEpsilon))
                {
                    // sweet - can reuse original slot
                }
                else
                {
                    bool match = false;

                    // already taken with a different value;
                    // see if there is already a duplicated vertex for this
                    // made for our new UV
                    for (int i=orig_num_vertices; i<(int)*num_vertices; i++)
                    {
                        // floating point numbers don't fare well in equality tests, so look for small diffs
                        if ((fabsf(vertices[0][i].x  - vertices[0][f_ptr->idx].x ) < kEpsilon) &&
                            (fabsf(vertices[0][i].y  - vertices[0][f_ptr->idx].y ) < kEpsilon) &&
                            (fabsf(vertices[0][i].z  - vertices[0][f_ptr->idx].z ) < kEpsilon) &&
                            (fabsf(vertices[0][i].nx - vertices[0][f_ptr->idx].nx) < kEpsilon) &&
                            (fabsf(vertices[0][i].ny - vertices[0][f_ptr->idx].ny) < kEpsilon) &&
                            (fabsf(vertices[0][i].nz - vertices[0][f_ptr->idx].nz) < kEpsilon) &&
                            (fabsf(vertices[0][i].u  - f_ptr->u) < kEpsilon) &&
                            (fabsf(vertices[0][i].v  - f_ptr->v) < kEpsilon))
                        {
                            // match - use it
                            match = true;
                            f_ptr->idx = i;
                            break;
                        }
                    }
                    if (!match)
                    {
                        (*num_vertices)++;
                        uv_assigned = (bool *)realloc( uv_assigned, sizeof(bool) * *num_vertices );

                        for (int j=0; j<num_frames; j++)
                        {
                            vertices[j] = (vertex_t *)realloc( vertices[j], sizeof(vertex_t) * *num_vertices );

                            vertices[j][*num_vertices-1].x  = vertices[j][f_ptr->idx].x;
                            vertices[j][*num_vertices-1].y  = vertices[j][f_ptr->idx].y;
                            vertices[j][*num_vertices-1].z  = vertices[j][f_ptr->idx].z;
                            vertices[j][*num_vertices-1].nx = vertices[j][f_ptr->idx].nx;
                            vertices[j][*num_vertices-1].ny = vertices[j][f_ptr->idx].ny;
                            vertices[j][*num_vertices-1].nz = vertices[j][f_ptr->idx].nz;
                            vertices[j][*num_vertices-1].u  = f_ptr->u;
                            vertices[j][*num_vertices-1].v  = f_ptr->v;
                        }
                        uv_assigned[*num_vertices-1] = true;

                        f_ptr->idx = *num_vertices-1;
                    }
                }
            }
        }
    }
    free( uv_assigned );
}

void Window3DNavigator::LoadMesh( SLFFile *f )
{
    auto* cache = GetSubsystem<ResourceCache>();

    vertex_t *v           = NULL;
    face_t   *p           = NULL;
    uv_t     *uv          = NULL;

    try
    {
        int32_t  has_sunlight     = f->readLELong();
        uint32_t num_vertices     = f->readLEULong();
        int32_t  uv_count         = f->readLELong();
        uint32_t num_polys        = f->readLEULong();

        f->skip(8);

        int32_t vertex_materials  = f->readLELong();

        // printf("%08x: %d vertices, %d triangles, uv_count=%d has_sunlight=%d, vertex_materials=%d\n", f->pos(), num_vertices, num_polys, uv_count, has_sunlight, vertex_materials);

        // Start off optimistic hoping we don't have to duplicate vertices; we will but nevermind...
        v = (vertex_t *)malloc( sizeof(vertex_t)*num_vertices );
        for (int k=0; k<(int)num_vertices; k++)
        {
            v[k].x = f->readFloat() * kScale;
            v[k].y = f->readFloat() * kScale;
            v[k].z = f->readFloat() * kScale;

            // Other fields get setup below
        }

        uv = (uv_t *)malloc(sizeof(uv_t)*uv_count);
        for (int k=0; k<uv_count; k++)
        {
            uv[k].u = f->readFloat();
            uv[k].v = f->readFloat();
        }

        // Skip vertex material array
        if (vertex_materials < 0)
        {
            f->skip( num_vertices * 4 );
        }

        p = (face_t *)malloc(sizeof(face_t)*num_polys);

        // poly uv array
        for (int k=0; k<(int)num_polys; k++)
        {
            for (int j=0; j<VERTICES_PER_FACE; j++)
            {
                uint32_t idx = f->readLELong();

                p[k].vert[j].u = uv[ idx ].u;
                p[k].vert[j].v = uv[ idx ].v;
            }
        }
        free( uv );

        // Poly vertex array
        for (int k=0; k<(int)num_polys; k++)
        {
            for (int j=0; j<VERTICES_PER_FACE; j++)
            {
                p[k].vert[j].idx = f->readLEULong();
            }
        }

        // Poly texture array
        // It is NOT safe to assume all polygons in a mesh use the same material; they don't
        for (int k=0; k<(int)num_polys; k++)
        {
            p[k].material_idx = f->readLELong();
        }

        // Vertex normal array
        for (int k=0; k<(int)num_vertices; k++)
        {
            v[k].nx = f->readFloat();
            v[k].ny = f->readFloat();
            v[k].nz = f->readFloat();
        }

        // Skip static lighting array - Urho places a limit of 4 per vertex lights per object
        // (if it is exceeded the brightest 4 on the object show) SetPerVertex() needs to be
        // called on the light
        f->skip( num_vertices * 12 );

        // Skip poly equation array - which I don't even know what is
        f->skip( num_polys * 16 );

        // Skip sunlight intensity array
        if ((has_sunlight >> 8) & 0xff)
        {
            f->skip( num_vertices * 4 );
        }

        // Make a list of all the Materials used in this mesh, so that
        // we can do all polygons of the one material in one go. We do
        // this to minimise the overhead of the rigidbody physics as
        // much as we can. Since we only do this within a single mesh,
        // we hope that the polygons aren't going to be spread out all
        // over the level where it makes a mockery of the bounding box.
        Vector<int32_t> collated_materials = CollateMaterials( p, (int)num_polys );

        // Now we double up any vertices that need to use
        // multiple uvs, and adjust the vertex indices
        // to point to newly added vertices

        duplicateVerticesForUVs( p, (int)num_polys, &v, &num_vertices, 1 );

        SharedPtr<VertexBuffer> vb(new VertexBuffer(context_));

        PODVector<VertexElement> elements;
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));
        elements.Push(VertexElement(TYPE_VECTOR2, SEM_TEXCOORD));

        // Though not necessary to render, the vertex & index buffers must be listed
        // in the model so that it can be saved properly
        Vector<SharedPtr<VertexBuffer> > vertexBuffers;

        // Shadowed buffer needed for raycasts to work, and so that data can be
        // automatically restored on device loss
        vb->SetShadowed( true );
        vb->SetSize( num_vertices, elements );
        vb->SetData( v );

        vertexBuffers.Push(vb);

        // We can't free the v list because vertexBuffers is maintaining a reference to it
        stuffToFreeLater_.Push( v );

        for (int i=0; i<(int)collated_materials.Size(); i++)
        {
            // We can only texture at the object level, not the geometry level,
            // so this forces us to make LOTS and LOTS of models to represent
            // the scene. Unfortunate because it means a lot more objects to
            // track with the physics engine too.

            SharedPtr<Model> fromScratchModel(new Model(context_));

            Vector<SharedPtr<IndexBuffer> > indexBuffers;

            fromScratchModel->SetNumGeometries(1);

            SharedPtr<IndexBuffer> ib(new IndexBuffer(context_));
            SharedPtr<Geometry> geom(new Geometry(context_));

            // Go through all the polygons in the mesh, and if they are
            // using the current texture, add their vertices into a new
            // list of indices. Also keep track of the minimum and maximum
            // cartesian co-ordinates so an appropriate bounding box can be
            // set.

            // v_indices are 32 bit because that's the way the PVL format stored them
            // and we can bulk read them that way, but Urrho3D wants uint16
            uint16_t *v_thisgeo = (uint16_t *)malloc(sizeof(uint16_t)*3*num_polys);
            int geo_upto = 0;

            // Set these to large values in the wrong direction so the first
            // point check will correct them
            Vector3 meshMin( 10000000.0, 10000000.0, 10000000.0) ;
            Vector3 meshMax(-10000000.0,-10000000.0,-10000000.0) ;

            for (int k=0; k<(int)num_polys; k++)
            {
                if (p[k].material_idx == collated_materials[i])
                {
                    for (int j=0; j<VERTICES_PER_FACE; j++)
                    {
                        v_thisgeo[geo_upto++] = p[k].vert[j].idx;

                        vertex_t *v_pt = &(v[ p[k].vert[j].idx ]);

                        if (meshMin.x_ > v_pt->x)
                            meshMin.x_ = v_pt->x;
                        if (meshMin.y_ > v_pt->y)
                            meshMin.y_ = v_pt->y;
                        if (meshMin.z_ > v_pt->z)
                            meshMin.z_ = v_pt->z;
                        if (meshMax.x_ < v_pt->x)
                            meshMax.x_ = v_pt->x;
                        if (meshMax.y_ < v_pt->y)
                            meshMax.y_ = v_pt->y;
                        if (meshMax.z_ < v_pt->z)
                            meshMax.z_ = v_pt->z;
                    }
                }
            }
            v_thisgeo = (uint16_t *) realloc(v_thisgeo, sizeof(uint16_t)*geo_upto);

            // Update the world bounding box
            if (worldMin_.x_ > meshMin.x_)
                worldMin_.x_ = meshMin.x_;
            if (worldMin_.y_ > meshMin.y_)
                worldMin_.y_ = meshMin.y_;
            if (worldMin_.z_ > meshMin.z_)
                worldMin_.z_ = meshMin.z_;
            if (worldMax_.x_ < meshMax.x_)
                worldMax_.x_ = meshMax.x_;
            if (worldMax_.y_ < meshMax.y_)
                worldMax_.y_ = meshMax.y_;
            if (worldMax_.z_ < meshMax.z_)
                worldMax_.z_ = meshMax.z_;

            ib->SetShadowed( true );
            ib->SetSize( geo_upto, false );
            ib->SetData( v_thisgeo );

            indexBuffers.Push(ib);

            // We can't free this list, because ib maintains a reference to it now
            stuffToFreeLater_.Push( v_thisgeo );

            geom->SetVertexBuffer( 0, vb );
            geom->SetIndexBuffer( ib );
            geom->SetDrawRange( TRIANGLE_LIST, 0, geo_upto );

            fromScratchModel->SetGeometry( 0, 0, geom );

            fromScratchModel->SetBoundingBox( BoundingBox(meshMin, meshMax) );

            PODVector<unsigned> morphRangeStarts;
            PODVector<unsigned> morphRangeCounts;
            morphRangeStarts.Push(0);
            morphRangeCounts.Push(0);
            fromScratchModel->SetVertexBuffers(vertexBuffers, morphRangeStarts, morphRangeCounts);
            fromScratchModel->SetIndexBuffers(indexBuffers);

            Node* objectNode = scene_->CreateChild();
            objectNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            auto* object = objectNode->CreateComponent<StaticModel>();
            object->SetModel(fromScratchModel);

            int material_idx = collated_materials[ i ];

            if (materials_[ material_idx ]->hash_ == "")
                material_idx = materials_[ material_idx ]->mapsTo_;

            object->SetMaterial( materials_[ material_idx ]->material_ );

            if (materials_[ material_idx ]->IsCollidable())
            {
                auto* body = objectNode->CreateComponent<RigidBody>();
                body->SetCollisionLayer(2);
                auto* shape = objectNode->CreateComponent<CollisionShape>();
                shape->SetTriangleMesh(object->GetModel(), 0);
            }
            if (materials_[ material_idx ]->IsWater())
            {
                // We have to duplicate the model, since we can't attach multiple textures
                // to the one model. The duplicated model will have a reflection surface on
                // it, and will be depth biased to be just beneath the translucent surface
                // of the original. (If we do this the other way around, the reflection is
                // the ONLY surface you see.)

                // But...
                // We can't just use the fromScratchModel we just created though, because
                // Wizardry double sides its water meshes - ie. it has each polygon listed
                // clockwise and anti-clockwise. We only want our reflection to be looking
                // down onto the water, not looking up, so we have to filter the polygons
                // to only include those with a normal facing upwards. This is necessary
                // because the surface reflecting back at you from underwater looks completely
                // wrong.

                // So the very first thing we have to do is filter our polygons to determine
                // if we even have any upward facing polygons at all before creating any models

                int filtered_polys = 0;
                uint16_t *v_geofiltered = (uint16_t *)malloc(sizeof(uint16_t)*geo_upto);
                for (int k=0; k<geo_upto; k+=3)
                {
                    Vector3& v1 = *(reinterpret_cast<Vector3*>(&(v[ v_thisgeo[k+0] ].x)));
                    Vector3& v2 = *(reinterpret_cast<Vector3*>(&(v[ v_thisgeo[k+1] ].x)));
                    Vector3& v3 = *(reinterpret_cast<Vector3*>(&(v[ v_thisgeo[k+2] ].x)));

                    Vector3 edge1 = v1 - v2;
                    Vector3 edge2 = v1 - v3;
                    if (edge1.CrossProduct(edge2).Normalized().y_ > 0.0)
                    {
                        // Polygon faces upwards
                        v_geofiltered[filtered_polys++] = v_thisgeo[k+0];
                        v_geofiltered[filtered_polys++] = v_thisgeo[k+1];
                        v_geofiltered[filtered_polys++] = v_thisgeo[k+2];
                    }
                }
                if (! filtered_polys)
                {
                    free( v_geofiltered );
                }
                else
                {
                    v_geofiltered = (uint16_t *) realloc(v_geofiltered, sizeof(uint16_t)*filtered_polys);

                    SharedPtr<Model> waterNormalUp(new Model(context_));

                    Vector<SharedPtr<IndexBuffer> > filteredIndexBuffers;

                    waterNormalUp->SetNumGeometries(1);

                    SharedPtr<IndexBuffer> wnib(new IndexBuffer(context_));
                    SharedPtr<Geometry> wngeom(new Geometry(context_));

                    wnib->SetShadowed( true );
                    wnib->SetSize( filtered_polys, false );
                    wnib->SetData( v_geofiltered );

                    filteredIndexBuffers.Push(wnib);

                    // We can't free this list, because wnib maintains a reference to it now
                    stuffToFreeLater_.Push( v_geofiltered );

                    wngeom->SetVertexBuffer( 0, vb );
                    wngeom->SetIndexBuffer( wnib );
                    wngeom->SetDrawRange( TRIANGLE_LIST, 0, filtered_polys );

                    waterNormalUp->SetGeometry( 0, 0, wngeom );

                    // Reusing bounding box from the original object rather than recalculate it
                    waterNormalUp->SetBoundingBox( BoundingBox(meshMin, meshMax) );

                    waterNormalUp->SetVertexBuffers(vertexBuffers, morphRangeStarts, morphRangeCounts);
                    waterNormalUp->SetIndexBuffers(filteredIndexBuffers);

                    SharedPtr<Material> waterMat = FindWaterMaterialPlaneAt( v[ v_geofiltered[0] ].y );

                    if (!waterMat)
                    {
                        // We have to clone the material, because we potentially have multiple
                        // pools of water, all at different y heights, and they all need to use
                        // their own individual reflection camera set at that plane. The cloning
                        // allows us to keep them separate.
                        waterMat = cache->GetResource<Material>("Materials/Water.xml")->Clone();

                        // Use a positive z bias to put it just below the surface of the water texture
                        // that came from the PVL file.
                        waterMat->SetDepthBias(BiasParameters(0.001f, 0.0f));
                        // The Water.xml file sets up a water noise texture that while more realistic
                        // doesn't actually look as nice for our usage with a blue water overlayed above it.
                        // Switch it off
                        waterMat->SetTexture(TU_NORMAL, NULL);

                        // printf("Creating new waterplane and reflection camera at y=%1.2f\n", v[ v_geofiltered[0] ].y );
                        SharedPtr<Texture2D> renderTexture = CreateReflectionTextureAtPlane( v[ v_geofiltered[0] ].y );
                        waterMat->SetTexture(TU_DIFFUSE, renderTexture);

                        AddWaterMaterialPlaneAt( v[ v_geofiltered[0] ].y, waterMat );
                    }
                    Node* waterNode = scene_->CreateChild();
                    waterNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
                    auto* water = waterNode->CreateComponent<StaticModel>();

                    water->SetModel(waterNormalUp);

                    water->SetMaterial( waterMat );
                    water->SetViewMask(0x80000000);
                    object->SetViewMask(0x80000000);
                }
            }
            else
            {
                object->SetCastShadows(true);
            }
        }
        collated_materials.Clear();

        if (p)         free(p);
    }
    catch (SLFFileException &e)
    {
        fprintf(stderr, "File too short at offset 0x%08x\n",(unsigned int)f->pos());
        if (p)         free(p);
        if (v)         free(v);
        throw SLFFileException();
    }
}

typedef struct
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float intensity;
    float radius;
} light_t;

void Window3DNavigator::LoadLights( SLFFile *f )
{
    int16_t   num_lights = f->readLEShort();

    for (int k=0; k<num_lights; k++)
    {
        int16_t record_type;
        uint8_t light_unknowns[6];
        light_t light_record;
        char    light_name[20];

        record_type = f->readLEShort();
        f->read( (char *)light_unknowns, sizeof(light_unknowns) );
        // scale the light the same as the scene (Wizardry 8 itself scaled the lights to 1/500th of
        // geometry, so apply that on top of ours)
        light_record.x         = f->readFloat() * 500.0 * kScale;
        light_record.y         = f->readFloat() * 500.0 * kScale;
        light_record.z         = f->readFloat() * 500.0 * kScale;
        light_record.r         = f->readFloat();
        light_record.g         = f->readFloat();
        light_record.b         = f->readFloat();
        light_record.intensity = f->readFloat() * kLightIntensityScale;
        light_record.radius    = f->readFloat() * kScale;

        if (record_type < 2)
        {
            light_unknowns[1] = 1;
            strcpy( light_name, "Static Point Light");
        }
        else
        {
            f->read( light_name, sizeof(light_name) );

            if (light_unknowns[2] & 2)
            {
                int8_t no_idea; // actually 60 bytes of unknown but we just skip the others

                light_unknowns[0] = 1;

                no_idea = f->readUByte();
                f->skip( 59 );
                // no_idea[8] can override light_record.intensity but only with a lot of other logic
                // not included here
                if (no_idea & 0x10)
                {
                    processProperties( f, false, NULL );
                }
            }
        }

        Node* lightNode = scene_->CreateChild();
        lightNode->SetPosition(Vector3( light_record.x, light_record.y, light_record.z));
        auto* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetColor(Color( light_record.r, light_record.g, light_record.b ));
        light->SetRadius( light_record.radius );
        light->SetBrightness( light_record.intensity );

        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
        light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
        light->SetSpecularIntensity(0.5f);
    }
}

void Window3DNavigator::SkipMonsters( SLFFile *f )
{
    int32_t num_monsters = f->readLELong();

    for (int k=0; k<num_monsters; k++)
    {
        char      monster_id[20];

        f->read( monster_id, sizeof(monster_id) );

        processProperties( f, false, NULL );
    }
}

// This is the same way Wizardry calculates it - with a 3x3 non-homogenous matrix
static void setupMatrix( float *matrix_33, float theta, float rotvec_x, float rotvec_y, float rotvec_z )
{
    matrix_33[0*3+0] = 1.0; // Identity matrix
    matrix_33[0*3+1] = 0.0;
    matrix_33[0*3+2] = 0.0;
    matrix_33[1*3+0] = 0.0;
    matrix_33[1*3+1] = 1.0;
    matrix_33[1*3+2] = 0.0;
    matrix_33[2*3+0] = 0.0;
    matrix_33[2*3+1] = 0.0;
    matrix_33[2*3+2] = 1.0;

    // This is setting up the standard 3d matrix for rotation around an arbitrary point
    if ( theta != 0.0 )
    {
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);

        matrix_33[0*3+0] = (1.0 - rotvec_x * rotvec_x) * cos_theta + rotvec_x * rotvec_x;
        matrix_33[0*3+1] = rotvec_y * rotvec_x * (1.0 - cos_theta) - rotvec_z * sin_theta;
        matrix_33[0*3+2] =  rotvec_y * sin_theta + rotvec_z * rotvec_x * (1.0 - cos_theta);

        matrix_33[1*3+0] =  rotvec_z * sin_theta + rotvec_y * rotvec_x * (1.0 - cos_theta);
        matrix_33[1*3+1] = (1.0 - rotvec_y * rotvec_y) * cos_theta + rotvec_y * rotvec_y;
        matrix_33[1*3+2] = (1.0 - cos_theta) * rotvec_z * rotvec_y - rotvec_x * sin_theta;

        matrix_33[2*3+0] = rotvec_z * rotvec_x * (1.0 - cos_theta) - rotvec_y * sin_theta;
        matrix_33[2*3+1] = rotvec_x * sin_theta + (1.0 - cos_theta) * rotvec_z * rotvec_y;
        matrix_33[2*3+2] = (1.0 - rotvec_z * rotvec_z) * cos_theta + rotvec_z * rotvec_z;
    }
}

static void multiplyVector( float *vector_3, float *matrix_33)
{
    float result[3];

    // You can't modify vector_3 until all the calculations are complete
    result[0] = matrix_33[0*3+0] * vector_3[0] + matrix_33[0*3+1] * vector_3[1] + matrix_33[0*3+2] * vector_3[2];
    result[1] = matrix_33[1*3+0] * vector_3[0] + matrix_33[1*3+1] * vector_3[1] + matrix_33[1*3+2] * vector_3[2];
    result[2] = matrix_33[2*3+0] * vector_3[0] + matrix_33[2*3+1] * vector_3[1] + matrix_33[2*3+2] * vector_3[2];

    vector_3[0] = result[0];
    vector_3[1] = result[1];
    vector_3[2] = result[2];
}

void Window3DNavigator::processProperties( SLFFile *f, bool expect_matrix, void *v )
{
    qint8 s = f->readByte();

    if (s == 0)
    {
        int8_t  m;
        int32_t num_frames;

        m = f->readByte();
        f->skip( 8 );
        num_frames = f->readLELong();

        if (! expect_matrix || (m != 2))
        {
            f->skip( 28 * num_frames );
            if (m == 2)
                f->skip( 12 * num_frames );
        }
        else
        {
            struct animated_mesh *ani_ptr = (struct animated_mesh *)v;

            // We need to make a copy of the first frame with the
            // vertices readjusted to remove the scale and the origin offset
            // because the first frame is about to be overwritten with
            // the matrix calculated version, and we will need this information
            // to calculate all the other frames also.

            uint32_t  num_vertices = ani_ptr->num_vertices;
            float    *vertices = (float *)malloc( sizeof(float)*3 * num_vertices );

            if (vertices)
            {
                for (int k=0; k<(int)num_vertices; k++)
                {
                    vertices[k*3+0] = ani_ptr->frames[ 0 ][ k ].x / 500.0 / kScale;
                    vertices[k*3+1] = ani_ptr->frames[ 0 ][ k ].y / 500.0 / kScale;
                    vertices[k*3+2] = ani_ptr->frames[ 0 ][ k ].z / 500.0 / kScale;
                }
            }

            for (int j=0; j<num_frames; j++)
            {
                float x = f->readFloat();
                float y = f->readFloat();
                float z = f->readFloat();

                float rotation = f->readFloat();
                float rotvec_x = f->readFloat();
                float rotvec_y = f->readFloat();
                float rotvec_z = f->readFloat();

                float u1 = f->readFloat();
                float u2 = f->readFloat();
                float u3 = f->readFloat();

                if (ani_ptr->frames && (ani_ptr->num_frames >= num_frames))
                {
                    if (!ani_ptr->frames[j])
                    {
                        ani_ptr->frames[j] = (vertex_t *)calloc( ani_ptr->num_vertices, sizeof(vertex_t) );
                    }
                    if (ani_ptr->frames[j])
                    {
                        float matrix[9];

                        setupMatrix( matrix, rotation, rotvec_x, rotvec_y, rotvec_z );

                        for (int k=0; k<(int)ani_ptr->num_vertices; k++)
                        {
                            ani_ptr->frames[ j ][ k ].x = vertices[k*3+0] - x;
                            ani_ptr->frames[ j ][ k ].y = vertices[k*3+1] - y;
                            ani_ptr->frames[ j ][ k ].z = vertices[k*3+2] - z;

                            multiplyVector( &(ani_ptr->frames[j][k].x), matrix );

                            ani_ptr->frames[ j ][ k ].x += ani_ptr->origin_x;
                            ani_ptr->frames[ j ][ k ].y += ani_ptr->origin_y;
                            ani_ptr->frames[ j ][ k ].z += ani_ptr->origin_z;

                            ani_ptr->frames[ j ][ k ].x *= 500.0 * kScale;
                            ani_ptr->frames[ j ][ k ].y *= 500.0 * kScale;
                            ani_ptr->frames[ j ][ k ].z *= 500.0 * kScale;

                            // Copy the UVs from the 0th frame
                            if (j > 0)
                            {
                                ani_ptr->frames[ j ][ k ].u = ani_ptr->frames[ 0 ][ k ].u;
                                ani_ptr->frames[ j ][ k ].v = ani_ptr->frames[ 0 ][ k ].v;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Window3DNavigator::SkipPropsOrBitmaps( SLFFile *f )
{
    int32_t num_things = f->readLELong();

    assert(num_things < 100000);
    for (int k=0; k < num_things; k++)
    {
        struct animated_mesh ani;

        uint8_t  sbuf1;
        uint8_t  sbuf2;
        uint8_t  sbuf3;

        memset(&ani, 0, sizeof(struct animated_mesh));

        sbuf2 = f->readUByte();
        if (sbuf2 > 3)
        {
            uint8_t pao[6];

            f->skip( 1 );
            if (sbuf2 >= 5)
            {
                f->skip( 1 );
                ani.origin_x = f->readFloat();
                ani.origin_y = f->readFloat();
                ani.origin_z = f->readFloat();
            }
            if (sbuf2 >= 6)
                f->skip( 4 );
            if (sbuf2 >= 7)
            {
                f->read( ani.object_name, 64 );
            }
            if (sbuf2 >= 8)
            {
                sbuf1 = f->readUByte();
                f->skip( 4*sbuf1 );
            }
            sbuf1 = f->readUByte();

            f->read( (char *)pao, 6 );
            if (sbuf1 >= 3)
            {
                ani.fps = f->readFloat();
            }
            if (sbuf1 >= 5)  f->skip( 1 );
            if (sbuf1 >= 11) f->skip( 1 );
            if (sbuf1 >= 6)  f->skip( 5 );
            f->skip( 50 );
            f->skip( pao[0] );
            if (sbuf1 >= 7)
            {
                ani.num_frames = f->readUByte();

                f->skip( 24 * ani.num_frames); // bounding boxes
            }
            if (sbuf1 >= 8)
            {
                uint8_t s, t;

                s = f->readUByte();
                for (int j=0; j<s; j++)
                {
                    t = f->readUByte();
                    f->skip( 32 );
                    if (t < 3)
                    {
                        if (t == 2)
                            f->skip( 60 );
                    }
                    else
                    {
                        t = f->readUByte();
                        if (t == 1)
                            f->skip( 60 );
                        if (t == 2)
                            f->skip( 129 );
                    }
                }
            }
            if (!pao[5])
            {
                if (sbuf1 >= 9)
                {
                    if (f->readByte())
                    {
                        processProperties(f, ani.expect_matrix, &ani);
                    }
                }
            }
            if (sbuf1 >= 10)
                f->skip( 1 );
            if (pao[5])
            {
                for (int z=0; z< pao[0]; z++)
                {
                    sbuf3 = f->readUByte();
                    for (int j=0; j<sbuf3; j++)
                    {
                        f->skip( 1 );

                        f->skip(2);
                        bitmapDetails( f, false, &ani );
                        processProperties(f, ani.expect_matrix, &ani);
                    }
                }
            }
            else
            {
                for (int j=0; j< pao[0]; j++)
                {
                    f->skip( 1 );
                    f->skip( 2 );
                    bitmapDetails( f, false, &ani );
                }
            }
        }
        else
        {
            f->skip( 3 );
            if (sbuf2 > 1)
                f->skip( 4 );

            f->skip(2);
            bitmapDetails( f, true, &ani );
        }
        if (sbuf2 >= 3)
        {
            if (f->readByte())
                processXRefs( f, ani.trigger_name, NULL );
        }
        if (sbuf2 >= 9)
        {
            if (f->readByte())
                f->skip( 2 );
        }
        if (! ani.trigger_name[0])
        {
            // object does NOT have a trigger on it - so should just be an
            // animated object without requiring any interaction.

            // Load the materials/textures for the mesh and get the index at which they're in the list now
            // The LoadMaterial() function won't load a completely identical material more than once, and
            // we have the mapsTo_ field of the materials_ list to backtrack to the actual load every time
            // a duplicate is encountered
            int startingMatIdx = -1;
            for (int k=0; k<(int)ani.num_materials; k++)
            {
                int idx = LoadMaterial( pvlFilename_.Substring(0, pvlFilename_.FindLast("/")), ani.material_buf[k] );

                if (startingMatIdx < 0)
                    startingMatIdx = idx;
            }
            if (startingMatIdx < 0)
                startingMatIdx = 0;

            Vector<int32_t> collated_materials = CollateMaterials( ani.faces, (int)ani.num_faces );

            // Pest: Copy the vertex_t for frame[0] so that the non-populated normals are absent
            // Have to do this because Urho doesn't give us a way to ignore a field, and Wizardry
            // decided not to put normals in on dynamic objects, but to include them on static ones.

            float *strippedFrame = (float *)malloc( 5 * sizeof(float) * ani.num_vertices );
            if (strippedFrame)
            {
                for (int k=0; k<(int)ani.num_vertices; k++)
                {
                    strippedFrame[5*k+0] = ani.frames[0][k].x;
                    strippedFrame[5*k+1] = ani.frames[0][k].y;
                    strippedFrame[5*k+2] = ani.frames[0][k].z;
                    strippedFrame[5*k+3] = ani.frames[0][k].u;
                    strippedFrame[5*k+4] = ani.frames[0][k].v;
                }
            }
            SharedPtr<VertexBuffer> vb(new VertexBuffer(context_));

            PODVector<VertexElement> elements;
            elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
            // No normals on these meshes
            elements.Push(VertexElement(TYPE_VECTOR2, SEM_TEXCOORD));

            // Though not necessary to render, the vertex & index buffers must be listed
            // in the model so that it can be saved properly
            Vector<SharedPtr<VertexBuffer> > vertexBuffers;

            // Shadowed buffer needed for raycasts to work, and so that data can be
            // automatically restored on device loss
            vb->SetShadowed( true );
            vb->SetSize( ani.num_vertices, elements );
            vb->SetData( strippedFrame );

            vertexBuffers.Push(vb);

            // We can't free the v list because vertexBuffers is maintaining a reference to it
            stuffToFreeLater_.Push( strippedFrame );

            for (int i=0; i<(int)collated_materials.Size(); i++)
            {
                // We can only texture at the object level, not the geometry level,
                // so this forces us to make LOTS and LOTS of models to represent
                // the scene. Unfortunate because it means a lot more objects to
                // track with the physics engine too.

                SharedPtr<Model> fromScratchModel(new Model(context_));

                Vector<SharedPtr<IndexBuffer> > indexBuffers;

                fromScratchModel->SetNumGeometries(1);

                SharedPtr<IndexBuffer> ib(new IndexBuffer(context_));
                SharedPtr<Geometry> geom(new Geometry(context_));

                // Go through all the polygons in the mesh, and if they are
                // using the current texture, add their vertices into a new
                // list of indices. Also keep track of the minimum and maximum
                // cartesian co-ordinates so an appropriate bounding box can be
                // set.

                // v_indices are 32 bit because that's the way the PVL format stored them
                // and we can bulk read them that way, but Urrho3D wants uint16
                uint16_t *v_thisgeo = (uint16_t *)malloc(sizeof(uint16_t)*3*ani.num_faces);
                int geo_upto = 0;

                // Set these to large values in the wrong direction so the first
                // point check will correct them
                Vector3 meshMin( 10000000.0, 10000000.0, 10000000.0) ;
                Vector3 meshMax(-10000000.0,-10000000.0,-10000000.0) ;

                // We're going to move the vertices around when we animate, so setup
                // some bounds that capture the scope of all frames combined.

                for (int k=0; k<(int)ani.num_faces; k++)
                {
                    if (ani.faces[k].material_idx == collated_materials[i])
                    {
                        for (int m=0; m<VERTICES_PER_FACE; m++)
                        {
                            v_thisgeo[geo_upto++] = ani.faces[k].vert[m].idx;

                            // Normally it would make more sense for the frames to be the
                            // outer level of the loop. But not on this occasion.
                            // We don't want v_thisgeo to skyrocket by recounting the same
                            // thing for multiple frames, and we've already filtered on
                            // material idx, making it more efficient to have this as the
                            // inner loop.

                            for (int j=0; j<(int)ani.num_frames; j++)
                            {
                                assert( ani.frames[j] );

                                vertex_t *v_pt = &(ani.frames[j][ ani.faces[k].vert[m].idx ]);

                                if (meshMin.x_ > v_pt->x)
                                    meshMin.x_ = v_pt->x;
                                if (meshMin.y_ > v_pt->y)
                                    meshMin.y_ = v_pt->y;
                                if (meshMin.z_ > v_pt->z)
                                    meshMin.z_ = v_pt->z;
                                if (meshMax.x_ < v_pt->x)
                                    meshMax.x_ = v_pt->x;
                                if (meshMax.y_ < v_pt->y)
                                    meshMax.y_ = v_pt->y;
                                if (meshMax.z_ < v_pt->z)
                                    meshMax.z_ = v_pt->z;
                            }
                        }
                    }
                }
                v_thisgeo = (uint16_t *) realloc(v_thisgeo, sizeof(uint16_t)*geo_upto);

                // Update the world bounding box
                if (worldMin_.x_ > meshMin.x_)
                    worldMin_.x_ = meshMin.x_;
                if (worldMin_.y_ > meshMin.y_)
                    worldMin_.y_ = meshMin.y_;
                if (worldMin_.z_ > meshMin.z_)
                    worldMin_.z_ = meshMin.z_;
                if (worldMax_.x_ < meshMax.x_)
                    worldMax_.x_ = meshMax.x_;
                if (worldMax_.y_ < meshMax.y_)
                    worldMax_.y_ = meshMax.y_;
                if (worldMax_.z_ < meshMax.z_)
                    worldMax_.z_ = meshMax.z_;

                ib->SetShadowed( true );
                ib->SetSize( geo_upto, false );
                ib->SetData( v_thisgeo );

                indexBuffers.Push(ib);

                // We can't free this list, because ib maintains a reference to it now
                stuffToFreeLater_.Push( v_thisgeo );

                geom->SetVertexBuffer( 0, vb );
                geom->SetIndexBuffer( ib );
                geom->SetDrawRange( TRIANGLE_LIST, 0, geo_upto );

                fromScratchModel->SetGeometry( 0, 0, geom );

                fromScratchModel->SetBoundingBox( BoundingBox(meshMin, meshMax) );

                PODVector<unsigned> morphRangeStarts;
                PODVector<unsigned> morphRangeCounts;
                morphRangeStarts.Push(0);
                morphRangeCounts.Push(0);
                fromScratchModel->SetVertexBuffers(vertexBuffers, morphRangeStarts, morphRangeCounts);
                fromScratchModel->SetIndexBuffers(indexBuffers);

                Node* objectNode = scene_->CreateChild();
                objectNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
                auto* object = objectNode->CreateComponent<StaticModel>();
                object->SetModel(fromScratchModel);

                // The StaticModel just created has no bones, so unless we want to create them on the
                // fly, an AnimationTrack isn't going to help here. So we've left this as a static
                // model that we'll need to keep updating manually.

                if (ani.num_frames > 1)
                {
                    float *exp_frames = (float *) malloc( ani.num_frames * ani.num_vertices * 5 * sizeof(float) );

                    if (exp_frames)
                    {
                        int num_frames = 0;

                        for (int j=0; j<(int)ani.num_frames; j++)
                        {
                            if (ani.frames[j])
                            {
                                for (int k=0; k<(int)ani.num_vertices; k++)
                                {
                                    exp_frames[ (num_frames * ani.num_vertices + k) * 5 + 0 ] = ani.frames[j][k].x;
                                    exp_frames[ (num_frames * ani.num_vertices + k) * 5 + 1 ] = ani.frames[j][k].y;
                                    exp_frames[ (num_frames * ani.num_vertices + k) * 5 + 2 ] = ani.frames[j][k].z;
                                    exp_frames[ (num_frames * ani.num_vertices + k) * 5 + 3 ] = ani.frames[j][k].u;
                                    exp_frames[ (num_frames * ani.num_vertices + k) * 5 + 4 ] = ani.frames[j][k].v;
                                }
                                num_frames++;
                            }
                        }
                        if (num_frames > 1)
                        {
                            auto* animator = objectNode->CreateComponent<WizardryAnimatedMesh>();

                            animator->num_frames_    = num_frames;
                            animator->num_vertices_  = ani.num_vertices;
                            animator->frames_        = exp_frames;      // will take ownership of memory
                            animator->next_frame_    = 1;               // 0 already setup here
                            animator->frame_delay_   = 1.0 / ani.fps;
                        }
                    }
                }

                int material_idx = collated_materials[ i ] + startingMatIdx;

                if (materials_[ material_idx ]->hash_ == "")
                    material_idx = materials_[ material_idx ]->mapsTo_;

                object->SetMaterial( materials_[ material_idx ]->material_ );

                if (materials_[ material_idx ]->IsCollidable())
                {
                    auto* body = objectNode->CreateComponent<RigidBody>();
                    body->SetCollisionLayer(2);
                    auto* shape = objectNode->CreateComponent<CollisionShape>();
                    shape->SetTriangleMesh(object->GetModel(), 0);
                }
                object->SetCastShadows(true);
            }
            collated_materials.Clear();

            // TODO:
            // add an animator that can apply the altered vertices over time
        }
        for (int k=0; k<(int)ani.num_materials; k++)
        {
            free( ani.material_buf[k] );
        }
        free( ani.material_buf );
        for (int k=0; k<(int)ani.num_frames; k++)
        {
            free( ani.frames[k] );
        }
        free( ani.frames );
        free( ani.faces );
    }
}

void Window3DNavigator::SkipCameras( SLFFile *f )
{
    int32_t num_cams = f->readLELong();

    assert(num_cams < 100000);

    for (int k=0; k<num_cams; k++)
    {
        int8_t   v8;

        f->skip( 8 );
        v8 = f->readByte();

        f->skip( 20 );

        if (v8)
            f->skip( 4 );

        processProperties(f, false, NULL);
    }
}

void Window3DNavigator::SkipItems( SLFFile *f )
{
    int32_t num_items = f->readLELong();

    for (int k=0; k<num_items; k++)
    {
        char      item_id[20];

        f->read( item_id, sizeof(item_id) );
        f->skip( 24 );

        if (f->readByte())
            processXRefs(f, NULL, NULL);

        f->skip( 19 );
    }
}

void Window3DNavigator::processXRefs( SLFFile *f, char *trigger_name_OUT, float *xyzh_OUT )
{
    int8_t    a,b,c,d;

    char    trigger_name[128];
    char    teleports_to[256];
    int32_t location_type;
    float   radius;

    a = f->readByte();
    b = f->readByte();

    switch (b)
    {
        default:
            break;

        case 1:
            a = f->readByte();
            f->skip( 542 );
            if (a > 1)
                f->skip( 68 );

            if (a > 2)
            {
                if (f->readByte())
                {
                    if (f->readByte())
                    {
                        f->skip( 153 );
                    }
                }
            }
            if (a > 3)
                f->skip( 4 );

            break;

        case 2:
            a = f->readByte();
            f->skip( 410 );
            if (a > 1)
                f->skip( 49 );

            if (a > 2)
                f->skip( 145 );
            if (a > 3)
                f->skip( 5 );

            if (a > 4)
            {
                if (f->readByte())
                {
                    if (f->readByte() == 2)
                    {
                        f->skip( 435 );
                    }
                }
            }
            break;

        case 3:
            a = f->readByte();
            f->skip( 196 );
            if (a >= 2)
                f->skip( 2 );

            if (a >= 3)
                f->skip( 40 );
            if (a >= 4)
            {
                f->read(trigger_name, 128);
                // If xyzh_OUT is null we're not hunting for teleports - we're hunting for
                // objects with names triggers, and want to know about all of them
                if ((xyzh_OUT == NULL) && (trigger_name_OUT != NULL))
                {
                    strcpy( trigger_name_OUT, trigger_name );
                }
            }
            if (a >= 5)
                f->skip( 1 );
            break;

        case 4: // Location
            a = f->readByte();

            f->read(trigger_name, 128);
            // If xyzh_OUT is null we're not hunting for teleports - we're hunting for
            // objects with names triggers, and want to know about all of them
            if ((xyzh_OUT == NULL) && (trigger_name_OUT != NULL))
            {
                strcpy( trigger_name_OUT, trigger_name );
            }

            c = f->readByte();
            f->skip( 6 );
            location_type = f->readLELong();
            //switch (location_type)
            //{
            //  default:         printf("          Type: %08x\n", location_type);    break;
            //  case 0x00000000: printf("          Type: Generic\n");                break;
            //  case 0x00000009: printf("          Type: Camera Route Activate\n");  break;
            //  case 0x00000023: printf("          Type: Damage\n");                 break;
            //  case 0x00000026: printf("          Type: Effect Health\n");          break;
            //  case 0x00000030: printf("          Type: Light Enable\n");           break;
            //  case 0x00000034: printf("          Type: Teleporter\n");             break;
            //  case 0x00000044: printf("          Type: Particle Enable\n");        break;
            //}

            f->skip( 8 );
            f->read(teleports_to, 256); // Teleports to
            //if (location_type == 0x34)
            //{
            //  if (teleports_to[0])
            //    printf("          Teleports to: %s\n", teleports_to);
            //  else
            //    printf("          Place of Arrival\n");
            //}
            f->skip( 391 );
            if (a >= 2)
                f->skip( 36 );
            f->skip( 780 );
            radius = f->readFloat(); // radius of the sphere
            (void)radius;
            if (a >= 3)
                f->skip( 128 );

            if (!(c & 1))
            {
                d = f->readByte();
                if (d == 1)
                {
                    float xyz[3];
                    float heading;
                    float dir_ref;

                    // Sphere location centre
                    xyz[0] = f->readFloat();
                    xyz[1] = f->readFloat();
                    xyz[2] = f->readFloat();
                    heading = f->readFloat();
                    f->skip(4);
                    dir_ref = f->readFloat();
                    f->skip(4);

                    if (trigger_name_OUT && xyzh_OUT)
                    {
                        if ((location_type == 0x34) && // Teleporter
                            (teleports_to[0] == 0))    // Place of arrival
                        {
                            strcpy( trigger_name_OUT, trigger_name );

                            xyzh_OUT[0] = xyz[0];
                            xyzh_OUT[1] = xyz[1];
                            xyzh_OUT[2] = xyz[2];

                            // kEpsilon is too small for the numbers that come up here.
                            // eg. the value -1.000069 occurs for -1.0 but that differs by way more than kEpsilon
                            //     from its target value
                            if      (fabsf(dir_ref - 1.0) < kSloppyEpsilon)     // if (dir_ref == 1.0)
                                xyzh_OUT[3] = heading;
                            else if (fabsf(dir_ref - 0.0) < kSloppyEpsilon)     // if (dir_ref == 0.0)
                                xyzh_OUT[3] = 0.0;
                            else if (fabsf(dir_ref + 1.0) < kSloppyEpsilon)     // if (dir_ref == -1.0)
                                xyzh_OUT[3] = -heading;
                        }
                    }
                }
                else if (d == 2)
                    f->skip( 48 );
                if (f->readByte())
                    f->skip( 133 );
            }
            if (f->readByte())
            {
                d = f->readByte();
                if (d == 1)
                    f->skip( 153 );
                else if (d == 2)
                    f->skip( 435 );
            }

            break;
    }
}

void Window3DNavigator::LoadTriggers( SLFFile *f )
{
    int32_t num_triggers;

    char    trigger_name[128];
    float   xyzh[4];

    num_triggers = f->readLELong();
    // printf("Num Triggers: %d\n", num_triggers);

    // clear existing arrival points list
    while (arrivalPts_)
    {
        struct location *a = arrivalPts_->next;

        free(arrivalPts_);
        arrivalPts_ = a;
    }
    struct location *pt_list = arrivalPts_;

    for (int k=0; k<num_triggers; k++)
    {
        trigger_name[0] = '\0';
        xyzh[0] = xyzh[1] = xyzh[2] = xyzh[3] = NAN;

        processXRefs( f, trigger_name, xyzh );

        if (trigger_name[0] && (xyzh[0] != NAN))
        {
            if (pt_list == NULL)
            {
                arrivalPts_ = (struct location *)malloc(sizeof(struct location));
                pt_list = arrivalPts_;
            }
            else
            {
                pt_list->next = (struct location *)malloc(sizeof(struct location));
                pt_list = pt_list->next;
            }

            strcpy(pt_list->name, trigger_name);
            pt_list->x       = xyzh[0];
            pt_list->y       = xyzh[1];
            pt_list->z       = xyzh[2];
            pt_list->heading = xyzh[3];
            pt_list->next    = NULL;
        }
    }
}

void Window3DNavigator::SkipFogOptions( SLFFile *f )
{
    int8_t  b = f->readByte(); // possibly is underwater status - recheck

    // We ignore all these and just hard-code our own preferences
    float   near_fog      = f->readFloat();
    float   far_fog       = f->readFloat();
    float   unknown       = f->readFloat();
    float   ambient_light = f->readFloat();
    float   view_distance = f->readFloat();

    (void)near_fog;
    (void)far_fog;
    (void)unknown;
    (void)ambient_light;
    (void)view_distance;

    b = f->readByte();

    if ((b == 1) || (b == 2))
        f->skip( 12 );
    if (b == 2)
        f->skip( 16 );

    b = f->readByte();
    if (b)
        f->skip( 768 );

    b = f->readByte();
    if (b)
        f->skip( 768 );
}

int Window3DNavigator::LoadPVL( int map_id )
{
    pvlFilename_ = ::toUrho(::getLevelFilename( map_id ));

    // The last argument to this constructor is supposed to be defaultable, but
    // the compiler is choosing the wrong constructor if we omit it here (due to
    // out usage of C Strings instead of QStrings, it matches the wrong thing).
    // And usage of the wrong constructor fails to load anything at all.
    SLFFile f( "LEVELS", "LEVELS.SLF", pvlFilename_.CString(), false );

    // The PVL file format may not have been deliberately designed to be obfuscated
    // - it could easily have been a lot worse - but it also wasn't designed to be
    // easy to parse by another utility that doesn't care about absolutely everything
    // the game needs to care about either.

    // It's true that all the fields are separated by 4 byte 0xffffffff sequences, but
    // unless you can guarantee that that value also never occurs INSIDE any of the
    // sections, you can't just seek to the next 0xffffffff to jump over any fields.

    // Most of the internal structure IS floating point fields, and those that aren't
    // DO tend to be single byte chars, but there are also occasional 2 and 4 byte ints
    // as well, and this makes it an unsafe assumption to rely on. The skip functions ended up
    // looking pretty horrible trying to deal with all the conditional logic to make
    // sure a section was read correctly. No promises I've got it right; some of it got
    // very complicated to follow.

    if (!f.isGood())
    {
        fprintf(stderr, "no good on %s (map:%d)\n", pvlFilename_.CString(), map_id);
        return -1;
    }
    f.open(QFile::ReadOnly);

    mapId_ = map_id;
    Urho3D::Log::Write(Urho3D::LOG_INFO, ::toUrho(::getLevelName( map_id )) + " --> " + pvlFilename_ );

    try
    {
        int32_t num_meshes    = f.readLELong();
        int32_t v22           = f.readLELong();
        int16_t num_materials = f.readLEShort();

        // printf("num_meshes=%d v22=%d num_materials=%d\n", num_meshes, v22, num_materials);
        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Materials");

        for (int k=0; k<(int)num_materials; k++)
        {
            uint8_t  type;
            uint8_t  material_buf[298];

            type = f.readUByte();

            if (type == 0x04)
            {
                if (298 - 1 != f.read((char *)material_buf, 298 - 1))
                    throw SLFFileException();
            }
            else
            {
                if (282 - 1 != f.read((char *)material_buf, 282 - 1))
                    throw SLFFileException();
            }
            LoadMaterial( pvlFilename_.Substring(0, pvlFilename_.FindLast("/")), material_buf );
        }

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // Set these to large values in the wrong direction so the first
        // point check will correct them
        worldMin_ = Vector3( 10000000.0, 10000000.0, 10000000.0) ;
        worldMax_ = Vector3(-10000000.0,-10000000.0,-10000000.0) ;

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Meshes");
        for (int j=0; j<num_meshes; j++)
        {
            LoadMesh( &f );

            if ((-1 != f.readLELong()))
            {
                fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
                return -2;
            }
        }
        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Lights");
        LoadLights( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Monsters");
        SkipMonsters( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Items");
        SkipItems( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Missiles");
        // Missiles is supposed to be a dud field
        int32_t junk = f.readLELong();

        assert(junk < 100000);

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Props");
        SkipPropsOrBitmaps( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Bitmaps");
        SkipPropsOrBitmaps( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Cameras");
        SkipCameras( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Fog Options");
        if (f.readLELong())
        {
            SkipFogOptions( &f);
        }

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("0x%08x: %s\n", (unsigned int)f.pos(), "Triggers");
        LoadTriggers( &f );

        if ((-1 != f.readLELong()))
        {
            fprintf(stderr, "Unexpected check byte at offset 0x%08x\n", (unsigned int)f.pos() - (unsigned int)sizeof(uint32_t));
            return -2;
        }

        // printf("done at offset 0x%08x\n", (unsigned int)f.pos());
    }
    catch (SLFFileException &e)
    {
        // Couldn't load file completely - failed at some point
        (void)e;
        fprintf( stderr, "Problem reading level file\n");
    }

    return 0;
}

void Window3DNavigator::UpdateAnimatedTextures(float timeStep)
{
    auto* cache = GetSubsystem<ResourceCache>();

    for (int k=0; k < (int)materials_.Size(); k++)
    {
        WizardryMaterial *m = materials_[k];

        if (m->hash_ == "")
        {
            // We only need to do each texture once
            continue;
        }

        if (m->textureNames_.Size() > 1)
        {
            m->textureUpdateCounter_ += timeStep;
            if (m->textureUpdateCounter_ > m->textureUpdateRate_)
            {
                m->textureUpdateCounter_ = 0;
                m->currentTextureIdx_ = (m->currentTextureIdx_+1) % m->textureNames_.Size();

                Texture2D *renderTexture = cache->GetResource<Texture2D>( m->textureNames_[ m->currentTextureIdx_ ] );

                m->material_->SetTexture(TU_DIFFUSE, renderTexture);
            }
        }
    }
}

void Window3DNavigator::CreateUI()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Set up global UI style into the root UI element
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    auto* window = new Window(context_);
    ui->GetRoot()->AddChild(window);
    window->SetMinSize((int)(1024.0 * hiDpiScale_), (int)(40.0 * hiDpiScale_));
    window->SetLayout(LM_HORIZONTAL, 6, IntRect((int)(6.0 * hiDpiScale_), (int)(6.0 * hiDpiScale_), (int)(6.0 * hiDpiScale_), (int)(6.0 * hiDpiScale_)));
    window->SetAlignment(HA_CENTER, VA_TOP);
    window->SetStyleAuto();

    Font *font  = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    // 2 state buttons
    auto* button = new Button(context_);
    if (button)
    {
        button->SetName("PositionInfo");
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);
        button->SetStyle("Button");

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Toggle Info");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonTogglePositionInfo));
    }
    button = new Button(context_);
    if (button)
    {
        button->SetName("Map");
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);
        button->SetStyle("Button");

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Toggle Map");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonToggleMap));
    }
    button = new Button(context_);
    if (button)
    {
        button->SetName("Noclip");
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);
        button->SetStyle("Button");

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Noclip Mode");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonToggleNoclip));
    }
    // Normal button
    button = new Button(context_);
    if (button)
    {
        button->SetStyleAuto();
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Position Jump");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonRespawn));
    }
    // Normal button
    button = new Button(context_);
    if (button)
    {
        button->SetStyleAuto();
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Change Map");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonChangeMap));
    }
    // Normal button
    button = new Button(context_);
    if (button)
    {
        button->SetStyleAuto();
        button->SetFixedWidth((int)(140.0 * hiDpiScale_));
        button->SetMinHeight((int)(24.0 * hiDpiScale_));
        button->SetFocusMode(FM_RESETFOCUS);

        auto* t = button->CreateChild<Text>();
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetFont(font, (int)(12.0 * hiDpiScale_));
        t->SetText("Save Position");

        window->AddChild(button);

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleButtonSavePosition));
    }
}

// FIXME: Not HiDPI scaled
void Window3DNavigator::CreateMapsWindow()
{
    auto* ui = GetSubsystem<UI>();

    // Create the Window and add it to the UI's root node
    windowMaps_ = new Window(context_);
    ui->GetRoot()->AddChild(windowMaps_);

    // Set Window size and layout settings
    windowMaps_->SetMinWidth(384);
    windowMaps_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    windowMaps_->SetAlignment(HA_CENTER, VA_CENTER);
    windowMaps_->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Available Maps");

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    windowMaps_->AddChild(titleBar);

    // Create a list.
    auto* list = windowMaps_->CreateChild<ListView>();
    //list->SetSelectOnClickEnd(true);
    list->SetName("Maps");
    list->SetHighlightMode(HM_ALWAYS);
    list->SetMinHeight(200);

    while (maps_)
    {
        struct map *m = maps_->next;

        free(maps_);
        maps_ = m;
    }
    struct map *map_list = maps_;

    const int *levels = ::getLevels();
    for (int k=0; levels[k] != -1; k++)
    {
        if (isSavePosition_)
        {
            // Level has to be in the list of visited levels
            bool visited = false;
            for (int j=0; visitedMaps_[j] != -1; j++)
            {
                if (visitedMaps_[j] == levels[k])
                {
                    visited = true;
                    break;
                }
            }
            if (!visited)
            {
                continue;
            }
        }
        if (map_list == NULL)
        {
            maps_ = (struct map *)malloc(sizeof(struct map));
            map_list = maps_;
        }
        else
        {
            map_list->next = (struct map *)malloc(sizeof(struct map));
            map_list = map_list->next;
        }
        map_list->mapId = levels[k];
        map_list->next  = NULL;
    }
    for (struct map *m = maps_; m; m=m->next)
    {
        Text* text = new Text(context_);
        text->SetAlignment(HA_LEFT, VA_TOP);
        text->SetStyle("FileSelectorListText");
        text->SetText( ::toUrho(::getLevelName(m->mapId)) );
        list->AddItem(text);
    }

    if (isSavePosition_)
    {
        Text* helpText = windowMaps_->CreateChild<Text>();
        helpText->SetAlignment(HA_LEFT, VA_CENTER);
        helpText->SetText("Only already visited maps are available for\nsave positions.");
    }

    // The OK button
    auto* button = new Button(context_);
    button->SetName("Button");
    button->SetMaxWidth(75);
    button->SetMinHeight(30);
    button->SetAlignment(HA_RIGHT, VA_TOP);

    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText("Ok");

    windowMaps_->AddChild(button);

    // Apply styles
    windowMaps_->SetStyleAuto();
    button->SetStyleAuto();
    buttonText->SetStyleAuto();
    list->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleMapsClosePressed));

    // Subscribe to either double clicks on the list itself, or a click followed by the ok button
    SubscribeToEvent(list, E_ITEMDOUBLECLICKED, URHO3D_HANDLER(Window3DNavigator, HandleMapChanged));
    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleMapChanged));
}

// FIXME: Not hiDPI scaled
void Window3DNavigator::CreateArrivalPointWindow()
{
    auto* ui = GetSubsystem<UI>();

    // Create the Window and add it to the UI's root node
    windowStartLocations_ = new Window(context_);
    ui->GetRoot()->AddChild(windowStartLocations_);

    // Set Window size and layout settings
    windowStartLocations_->SetMinWidth(384);
    windowStartLocations_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    windowStartLocations_->SetAlignment(HA_CENTER, VA_CENTER);
    windowStartLocations_->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText(::toUrho(::getLevelName( mapId_ )) + " Starting Positions");

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    windowStartLocations_->AddChild(titleBar);

    // Create a list.
    auto* list = windowStartLocations_->CreateChild<ListView>();
    //list->SetSelectOnClickEnd(true);
    list->SetName("Locations");
    list->SetHighlightMode(HM_ALWAYS);
    list->SetMinHeight(200);

    for (struct location *a = arrivalPts_; a; a=a->next)
    {
        Text* text = new Text(context_);
        text->SetAlignment(HA_LEFT, VA_TOP);
        text->SetStyle("FileSelectorListText");
        text->SetText(a->name);
        list->AddItem(text);
    }

    // The OK button
    auto* button = new Button(context_);
    button->SetName("Button");
    button->SetMaxWidth(75);
    button->SetMinHeight(30);
    button->SetAlignment(HA_RIGHT, VA_TOP);

    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    //buttonText->SetFont(font, (int)(12.0 * hiDpiScale_));
    buttonText->SetText("Ok");

    windowStartLocations_->AddChild(button);

    // Apply styles
    windowStartLocations_->SetStyleAuto();
    button->SetStyleAuto();
    buttonText->SetStyleAuto();
    list->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleLocationsClosePressed));

    // Subscribe to either double clicks on the list itself, or a click followed by the ok button
    SubscribeToEvent(list, E_ITEMDOUBLECLICKED, URHO3D_HANDLER(Window3DNavigator, HandleLocationChanged));
    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(Window3DNavigator, HandleLocationChanged));
}

void Window3DNavigator::HandleLocationChanged(StringHash eventType, VariantMap& eventData)
{
    unsigned item = M_MAX_UNSIGNED;

    if (eventType == E_ITEMDOUBLECLICKED)
    {
        item = eventData[ItemDoubleClicked::P_SELECTION].GetInt();
    }
    else if (eventType == E_RELEASED)
    {
        ListView* list = (ListView *)windowStartLocations_->GetChild("Locations", false);
        item = list->GetSelection();
    }

    if (item != M_MAX_UNSIGNED)
    {
        unsigned i=0;
        for (struct location *a = arrivalPts_; a; a=a->next)
        {
            if (i == item)
            {
                SetAvatarPosition(Vector3( a->x, a->y, a->z) * 500.0, a->heading);
                break;
            }
            i++;
        }
        ShowLocationsWindow( false, true );
    }
}

void Window3DNavigator::HandleMapChanged(StringHash eventType, VariantMap& eventData)
{
    unsigned item = M_MAX_UNSIGNED;

    if (eventType == E_ITEMDOUBLECLICKED)
    {
        item = eventData[ItemDoubleClicked::P_SELECTION].GetInt();
    }
    else if (eventType == E_RELEASED)
    {
        ListView* list = (ListView *)windowMaps_->GetChild("Maps", false);
        item = list->GetSelection();
    }

    if (item != M_MAX_UNSIGNED)
    {
        unsigned i=0;

        for (struct map *m = maps_; m; m=m->next)
        {
            if (i == item)
            {
                Input* input         = GetSubsystem<Input>();

                input->SetMouseMode( MM_ABSOLUTE );
                input->SetMouseVisible( true );

                newMap_ = true;
                mapId_  = m->mapId;

                engine_->Exit(); // expectation is on the caller to reinstantiate this class
                return;
            }
            i++;
        }
        ShowMapsWindow( false, true );
    }
}

void Window3DNavigator::HandleLocationsClosePressed(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ShowLocationsWindow( false, false );
}

void Window3DNavigator::HandleMapsClosePressed(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ShowMapsWindow( false, false );
}

void Window3DNavigator::ShowDialog( bool show, bool lock_mouse )
{
    Input* input         = GetSubsystem<Input>();
    Node*  characterNode = scene_->GetChild("Jack", true);

    if (show == true)
    {
        input->SetMouseMode( MM_ABSOLUTE );
        input->SetMouseVisible( true );

        // turn off gravity temporarily, so we don't possibly have an endlessly
        // falling character while we choose a starting location
        if (characterNode)
        {
            auto* body = characterNode->GetComponent<RigidBody>();
            body->SetUseGravity(false);
        }
        UnsubscribeFromEvent(E_UPDATE);
        UnsubscribeFromEvent(E_POSTUPDATE);
    }
    else
    {
        if (lock_mouse)
        {
            input->SetMouseMode( MM_RELATIVE );
        }

        // turn gravity back on
        if (characterNode && !noclipActive_)
        {
            auto* body = characterNode->GetComponent<RigidBody>();
            body->SetUseGravity(true);
        }
        SubscribeToEvents();
    }
}

void Window3DNavigator::ShowLocationsWindow( bool show, bool lock_mouse )
{
    ShowDialog( show, lock_mouse );
    windowStartLocations_->SetVisible( show );
}

void Window3DNavigator::ShowMapsWindow( bool show, bool lock_mouse )
{
    ShowDialog( show, lock_mouse );
    windowMaps_->SetVisible( show );
}

void Window3DNavigator::CreateScene(int map_id)
{
    auto* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create scene subsystem components
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();

    auto* graphics = GetSubsystem<Graphics>();
    auto* renderer = GetSubsystem<Renderer>();


    // Front facing camera, and overhead map
    renderer->SetNumViewports( mapVisible_ ? 2 : 1 );

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    cameraNode_ = new Node(context_);
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    renderer->SetViewport(0, new Viewport(context_, scene_, camera));

// Don't think either of these add anything to the Wizardry scenes

//    SharedPtr<RenderPath> effectRenderPath = renderer->GetViewport(0)->GetRenderPath()->Clone();
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
//    // Make the bloom mixing parameter more pronounced
//    effectRenderPath->SetShaderParameter("BloomMix", Vector2(0.9f, 0.6f));
//    effectRenderPath->SetEnabled("Bloom", true);//false);
//    effectRenderPath->SetEnabled("FXAA2", true);//false);
//    renderer->GetViewport(0)->SetRenderPath(effectRenderPath);

    // Don't make this a child of cameraNode, because otherwise it tracks with the character movement,
    // which we don't want here. Set it in the dead middle of the map, facing downwards
    mapCameraNode_ = new Node(context_);

    auto *mapCamera = mapCameraNode_->CreateComponent<Camera>();
    mapCamera->SetViewOverrideFlags(VO_DISABLE_OCCLUSION | VO_DISABLE_SHADOWS);
    mapCamera->SetFarClip(1000000.0);
    mapCamera->SetOrthographic(true);
    // Face the map camera DOWN
    mapCameraNode_->Rotate(Quaternion(90.0f, Vector3::RIGHT));

    if (mapVisible_)
    {
        // The rect used by this measures LTRB _not_ LTWH
        // printf("[ %d %d %d %d]\n", (int)(graphics->GetWidth()  * 64 / 100),
        //              (int)(graphics->GetHeight() *  4 / 100),
        //              (int)(graphics->GetWidth()  * 97 / 100),
        //              (int)(graphics->GetHeight() * 37 / 100));
        renderer->SetViewport(1, new Viewport(context_, scene_, mapCamera,
            IntRect( graphics->GetWidth()  * 64 / 100,
                     graphics->GetHeight() *  4 / 100,
                     graphics->GetWidth()  * 97 / 100,
                     graphics->GetHeight() * 37 / 100)));
    }

    LoadPVL( map_id );

    camera->SetNearClip( M_MIN_NEARCLIP );
    //camera->SetViewOverrideFlags(VO_DISABLE_OCCLUSION);
    //camera->SetUseClipping(false);
//      auto* graphics = GetSubsystem<Graphics>();
//      graphics->SetCullMode(CULL_NONE);
// FIXME: Very bumpy in 1st person mode
// FIXME: Bit too eager to jump in 3rd person mode
// FIXME: have to jump at cemetary
// FIXME: map viewport in either overhead or cross section based on player height mode

// FIXME: no water in the fountains in Arnika, but awesomely the marble in the temple does reflect!
// FIXME: No ocean at Bayjin or Seacaves
// FIXME: No lava in Rapax Rift
// FIXME: Cosmic circle  sky should be inverted
// FIXME: _sometimes_ can't start unless in noclip - has happened in Swamp, Northern Wilderness and Cosmic Circle
// FIXME: Bridge in Northern Wilderness looks shockingly odd due to alpha on beams
// FIXME: LunarLegion hologram in Arnika doesn't render - looks like stain glass light. Plus can't walk through it.
    // Create static scene content. First create a zone for ambient lighting and fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.45f, 0.45f, 0.45f)); // I raised this from 0.15 in the example code because the shadows were too harsh ; There is a shadow intensity control that should probably be adjusted above it's default of 0.0 though instead
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    // Create a directional light with cascaded shadow mapping

    // Create out own sun directional light, because the embedded sun isn't sufficient
    // Suspect this was only ambient light, and that Wizardry 8 dynamically created the
    // actual sunlight based on time of day or night rather than store it in the level.
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    // Create skybox. The Skybox component is used like StaticModel,
    // but it will be always located at the camera, giving the
    // illusion of the box planes being far away. Use just the ordinary
    // Box model and a suitable material, whose shader will
    // generate the necessary 3D texture coordinates for cube mapping
    Node* skyNode = scene_->CreateChild("Sky");
    skyNode->SetScale(500.0f); // The scale actually does not matter
    auto* skybox = skyNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
}

SharedPtr<Texture2D> Window3DNavigator::CreateReflectionTextureAtPlane(float y)
{
    // Create a mathematical plane to represent the water in calculations
    Plane waterPlane = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, y, 0.0f));
    // Create a downward biased plane for reflection view clipping. Biasing is necessary to avoid too aggressive clipping
    Plane waterClipPlane = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, y-0.01f, 0.0f));

    // Create camera for water reflection
    // It will have the same farclip and position as the main viewport camera, but uses a reflection plane to modify
    // its position when rendering
    auto* graphics = GetSubsystem<Graphics>();
    Node *reflectionCameraNode = cameraNode_->CreateChild();
    auto* reflectionCamera = reflectionCameraNode->CreateComponent<Camera>();
    reflectionCamera->SetFarClip(300.0);
    reflectionCamera->SetViewMask(0x7fffffff); // Hide objects with only bit 31 in the viewmask (the water plane)
    reflectionCamera->SetAutoAspectRatio(false);
    reflectionCamera->SetUseReflection(true);
    reflectionCamera->SetReflectionPlane(waterPlane);
    reflectionCamera->SetUseClipping(true); // Enable clipping of geometry behind water plane
    reflectionCamera->SetClipPlane(waterClipPlane);
    // The water reflection texture is rectangular. Set reflection camera aspect ratio to match
    reflectionCamera->SetAspectRatio((float)graphics->GetWidth() / (float)graphics->GetHeight());
    // View override flags could be used to optimize reflection rendering. For example disable shadows
    //reflectionCamera->SetViewOverrideFlags(VO_DISABLE_SHADOWS);

    // Create a texture and setup viewport for water reflection. Assign the reflection texture to the diffuse
    // texture unit of the water material
    int texSize = 1024;
    SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
    renderTexture->SetSize(texSize, texSize, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
    renderTexture->SetFilterMode(FILTER_BILINEAR);
    RenderSurface* surface = renderTexture->GetRenderSurface();
    SharedPtr<Viewport> rttViewport(new Viewport(context_, scene_, reflectionCamera));
    surface->SetViewport(0, rttViewport);

    return renderTexture;
}

void Window3DNavigator::AddWaterMaterialPlaneAt(float y, SharedPtr<Material> m)
{
    // This isn't entirely suitable for a QMap because we have a floating point
    // index as a key, that needs to be treated with a range of tolerance.
    WizardryMaterial  *wm = new WizardryMaterial("ignore_hash");

    wm->isWater_  = true;
    wm->y_        = y;
    wm->material_ = m;

    waterPlanes_.Push( wm );
}

SharedPtr<Material> Window3DNavigator::FindWaterMaterialPlaneAt(float y)
{
    for (int k=0; k<(int)waterPlanes_.Size(); k++)
    {
        if (fabsf(waterPlanes_[k]->y_ - y) < 1.0)
            return waterPlanes_[k]->material_;
    }
    return NULL;
}

void Window3DNavigator::SetAvatarPosition(const Vector3& position, const float heading)
{
    Node* objectNode = scene_->GetChild("Jack", true);

    // The arrival teleports aren't actually on ground level.
    // So the character is going to drop slightly when they are used, even after applying the offset
    // difference between our Avatar and the Wizardry co-ordinate system.

    objectNode->SetPosition((position - Vector3( 0.0f, kAvatarOffset,  0.0f)) * kScale);

    character_ = objectNode->GetComponent<Avatar>();

    character_->controls_.yaw_ = heading / M_PI * 180.0;
    character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));
}

void Window3DNavigator::getPosition(float *x, float *y, float *z)
{
    Node*  characterNode = scene_->GetChild("Jack", true);

    Vector3 charPos = characterNode->GetWorldPosition();

    *x = charPos.x_ / kScale;
    // The + 10.0 is to try and ensure we stay above ground level, when this gets loaded in
    // the game itself. Getting the y co-ordinate to be compatible with the original game
    // has been a lot harder than the x or z ones.
    *y = charPos.y_ / kScale + kAvatarOffset + 10.0;
    *z = charPos.z_ / kScale;
}

float Window3DNavigator::getHeading()
{
    if (heading_ < 0)
        return 2 * M_PI + heading_;

    return heading_;
}

void Window3DNavigator::CreateAvatar()
{
    auto* cache = GetSubsystem<ResourceCache>();

    Node* objectNode = scene_->CreateChild("Jack");

    objectNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

    // spin node
    Node* adjustNode = objectNode->CreateChild("AdjNode");
    adjustNode->SetRotation( Quaternion(180, Vector3(0,1,0) ) );

    // Create the rendering component + animation controller
    auto* object = adjustNode->CreateComponent<AnimatedModel>();
    object->SetModel(cache->GetResource<Model>("Models/Mutant/Mutant.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Models/Mutant/Materials/mutant_M.xml"));
    object->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Set the head bone for manual control
    object->GetSkeleton().GetBone("Mutant:Head")->animated_ = false;

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    auto* body = objectNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(1);
    body->SetMass(1.0f);

    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body->SetAngularFactor(Vector3::ZERO);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    // Set a capsule shape for collision
    auto* shape = objectNode->CreateComponent<CollisionShape>();
    shape->SetCapsule( /*diameter*/ 0.7f, /*height*/ 1.4f, /*offset*/ Vector3(0.0f, 0.7f, 0.0f));

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    character_ = objectNode->CreateComponent<Avatar>();

    character_->controls_.yaw_ = 0.0 / M_PI * 180.0;
    character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));
    // Create some text representing the current character position.
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    positionText_ = ui->GetRoot()->CreateChild<Text>();
    positionText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), (int)(12.0 * hiDpiScale_));
    positionText_->SetTextAlignment(HA_LEFT);

    // Position the text relative to the screen center
    positionText_->SetHorizontalAlignment(HA_LEFT);
    positionText_->SetVerticalAlignment(VA_BOTTOM);
    positionText_->SetPosition( ui->GetRoot()->GetWidth() / 25,  -ui->GetRoot()->GetHeight() / 25);
    positionText_->SetVisible( positionInfoVisible_ );

    AddTgaToResourceCache( "DATA", "DATA.SLF", "AUTOMAP/MAP_PARTYMARKER_A.TGA" );
    // Create a marker sprite to show the current position on the map viewport.
    auto* markerTex = cache->GetResource<Texture2D>("AUTOMAP/MAP_PARTYMARKER_A.TGA");

    mapPositionMarker_ = new Sprite(context_);
    mapPositionMarker_->SetTexture(markerTex);
    mapPositionMarker_->SetSize(markerTex->GetWidth(), markerTex->GetHeight());
    mapPositionMarker_->SetHotSpot(32,32); // centre of the marker
    mapPositionMarker_->SetScale(0.5 * hiDpiScale_);
    mapPositionMarker_->SetOpacity(0.8f);
    mapPositionMarker_->SetPriority(100);

    ui->GetRoot()->AddChild(mapPositionMarker_);
    if (!mapVisible_)
    {
        mapPositionMarker_->SetVisible(false);
    }
}

void Window3DNavigator::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    auto* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText(
        "Use Arrow keys and mouse/touch to move\n"
        "Space to jump, F to toggle 1st/3rd person\n"
        "F5 to save scene, F7 to load"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

void Window3DNavigator::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Window3DNavigator, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Window3DNavigator, HandlePostUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void Window3DNavigator::HandleUpdate(StringHash /* eventType */, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();

    UpdateAnimatedTextures(timeStep);

    auto* input = GetSubsystem<Input>();

    if (character_)
    {
        // Clear previous controls
        character_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_STRAFE_LEFT | CTRL_STRAFE_RIGHT | CTRL_JUMP | CTRL_RUN, false);

        // Update controls using touch utility class
        if (touch_)
            touch_->UpdateTouches(character_->controls_);

        // Update controls using keys
        auto* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            if (!touch_ || !touch_->useGyroscope_)
            {
                character_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_UP));
                character_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_DOWN));
                character_->controls_.Set(CTRL_STRAFE_LEFT, input->GetKeyDown(KEY_COMMA));   // ,
                character_->controls_.Set(CTRL_STRAFE_RIGHT, input->GetKeyDown(KEY_PERIOD)); // .
                character_->controls_.Set(CTRL_RUN, input->GetKeyDown(KEY_LSHIFT));
                if (input->GetKeyDown(KEY_LEFT))
                    character_->controls_.yaw_ -= 750.0f * timeStep * YAW_SENSITIVITY;
                if (input->GetKeyDown(KEY_RIGHT))
                    character_->controls_.yaw_ += 750.0f * timeStep * YAW_SENSITIVITY;
            }
            character_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            if (input->IsMouseGrabbed())
            {
                // FIXME: We should be able to use GetMouseButtonPress() here instead, but it
                // hardly ever gets through - in contrast to the keyboard keys which do.
                // This is easy enough to see by putting debug in here for when it is set.
                // So instead using GetMouseButtonDown(), which isn't really what we want.
                character_->controls_.Set(CTRL_PUNCH, input->GetMouseButtonDown(MOUSEB_LEFT));
                character_->controls_.Set(CTRL_SWIPE, input->GetMouseButtonDown(MOUSEB_MIDDLE));
                character_->controls_.Set(CTRL_KICK,  input->GetMouseButtonDown(MOUSEB_RIGHT));

                character_->controls_.yaw_ += (float)input->GetMouseMoveX() * 0.1f * YAW_SENSITIVITY;
                character_->controls_.pitch_ += (float)input->GetMouseMoveY() * 0.1f * YAW_SENSITIVITY;
            }

            // Limit pitch
//            character_->controls_.pitch_ = Clamp(character_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));


            // Turn on/off gyroscope on mobile platform
            if (touch_ && input->GetKeyPress(KEY_G))
                touch_->useGyroscope_ = !touch_->useGyroscope_;

            #if 0
            if (input->GetKeyPress(KEY_F7))
            {
                File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/Window3DNavigator.xml", FILE_READ);
                scene_->LoadXML(loadFile);
                // After loading we have to reacquire the weak pointer to the Character component, as it has been recreated
                // Simply find the character's scene node by name as there's only one of them
                Node* characterNode = scene_->GetChild("Jack", true);
                if (characterNode)
                    character_ = characterNode->GetComponent<Avatar>();
            }
            #endif
        }
        /* Raycast down to get the floor normal under the character and store the normal value. */
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray( character_->GetNode()->GetPosition() , Vector3(0.0f,-1.0f,0.0f) ), 5.0f, 2);
        if (result.body_) {
            character_->floorNormal_ = result.normal_;
        }
    }
}

void Window3DNavigator::HandlePostUpdate(StringHash /* eventType */, VariantMap& /* eventData */)
{
    if (!character_)
        return;

    Node* characterNode = character_->GetNode();

    // Get camera lookat dir from character yaw + pitch
    const Quaternion& rot = characterNode->GetRotation();
    Quaternion dir = rot * Quaternion(character_->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("Mutant:Head", true);
    float limitPitch = character_->controls_.pitch_;//Clamp(character_->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));

    if (firstPerson_)
    {
        cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
        cameraNode_->SetRotation(dir);
    }
    else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = /*touch_ ? touch_->cameraDistance_ :*/ kCameraInitialDistance;
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, kCameraMinDistance, kCameraMaxDistance);

        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
        cameraNode_->SetRotation(dir);

    }

    Vector3 charPos = characterNode->GetWorldPosition();

    // update the map position marker on the map
    if (mapVisible_)
    {
        // If the entire map can fit in the viewport at the given zoom level, then centre it;
        // if not then set up a roving window that keeps the character in the centre of the screen
        // as far as possible, but starts letting the pointer move instead as they approach the edges
        // of the map, keeping the amount of map on viewport maximised at all times.
        // FIXME: This needs some edge case handling if they go into no clip mode and exit the edge of the map
        auto* mapCamera = mapCameraNode_->GetComponent<Camera>();
        auto* renderer  = GetSubsystem<Renderer>();
        mapCamera->SetZoom( 0.1 );//Min(map_width, map_height) );

        Frustum f = mapCamera->GetFrustum();

        Vector3 mapMin( 10000000.0, 10000000.0, 10000000.0) ;
        Vector3 mapMax(-10000000.0,-10000000.0,-10000000.0) ;

        for (int k=0; k<(int)NUM_FRUSTUM_VERTICES; k++)
        {
            if (mapMin.x_ > f.vertices_[k].x_)
                mapMin.x_ = f.vertices_[k].x_;
            if (mapMin.y_ > f.vertices_[k].y_)
                mapMin.y_ = f.vertices_[k].y_;
            if (mapMin.z_ > f.vertices_[k].z_)
                mapMin.z_ = f.vertices_[k].z_;
            if (mapMax.x_ < f.vertices_[k].x_)
                mapMax.x_ = f.vertices_[k].x_;
            if (mapMax.y_ < f.vertices_[k].y_)
                mapMax.y_ = f.vertices_[k].y_;
            if (mapMax.z_ < f.vertices_[k].z_)
                mapMax.z_ = f.vertices_[k].z_;
        }

        // FIXME: lot of floating point arithmetic here that shouldn't need to be done on every frame!
        float mapWidth    = mapMax.x_ - mapMin.x_;
        float mapHeight   = mapMax.z_ - mapMin.z_;
        float worldWidth  = worldMax_.x_ - worldMin_.x_;
        float worldHeight = worldMax_.z_ - worldMin_.z_;

        if ((mapWidth  >= worldWidth) &&
            (mapHeight >= worldHeight))
        {
            // map can be fit entirely within the viewport, set the position to be dead centre of the world
            mapCameraNode_->SetPosition( Vector3( (worldMin_.x_ + worldMax_.x_) / 2, worldMax_.y_, (worldMin_.z_ + worldMax_.z_) / 2 ));
        }
        else
        {
            Vector3 mapCentre( charPos.x_, worldMax_.y_, charPos.z_ );

            if (charPos.x_   >= worldMax_.x_ - mapWidth/2)
                mapCentre.x_ =  worldMax_.x_ - mapWidth/2;
            if (charPos.x_   <= worldMin_.x_ + mapWidth/2)
                mapCentre.x_ =  worldMin_.x_ + mapWidth/2;
            if (charPos.z_   >= worldMax_.z_ - mapHeight/2)
                mapCentre.z_ =  worldMax_.z_ - mapHeight/2;
            if (charPos.z_   <= worldMin_.z_ + mapHeight/2)
                mapCentre.z_ =  worldMin_.z_ + mapHeight/2;

            mapCameraNode_->SetPosition( mapCentre );
        }

        IntVector2 mapMarkerPos = renderer->GetViewport(1)->WorldToScreenPoint( charPos );
        mapPositionMarker_->SetPosition( mapMarkerPos.x_, mapMarkerPos.y_ );
        mapPositionMarker_->SetRotation( dir.YawAngle() );
    }
    heading_ = dir.YawAngle() / 180.0 * M_PI;

    positionText_->SetText( String("<") + String(charPos.x_ / kScale) + String(", ") +
                                          String(charPos.y_ / kScale) + String(", ") +
                                          String(charPos.z_ / kScale) + String(">")  +
                            String(" Facing: ") + String( heading_ ) +
                            String(" Pitch: ")  + String( dir.PitchAngle() ));
}

void Window3DNavigator::InitTouchInput()
{
    touchEnabled_ = true;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Input* input = GetSubsystem<Input>();
    XMLFile* layout = cache->GetResource<XMLFile>("UI/ScreenJoystick_Window3DNavigators.xml");
    screenJoystickIndex_ = (unsigned)input->AddScreenJoystick(layout, cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
    input->SetScreenJoystickVisible(screenJoystickSettingsIndex_, true);
}

void Window3DNavigator::HandleSceneUpdate(StringHash /*eventType*/, VariantMap& /* eventData */)
{
    // Move the camera by touch, if the camera node is initialized by descendant sample class
    if (touchEnabled_ && cameraNode_)
    {
        Input* input = GetSubsystem<Input>();
        for (unsigned i = 0; i < input->GetNumTouches(); ++i)
        {
            TouchState* state = input->GetTouch(i);
            if (!state->touchedElement_)    // Touch on empty space
            {
                if (state->delta_.x_ ||state->delta_.y_)
                {
                    Camera* camera = cameraNode_->GetComponent<Camera>();
                    if (!camera)
                        return;

                    Graphics* graphics = GetSubsystem<Graphics>();
                    yaw_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.x_;
                    pitch_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.y_;

                    // Construct new orientation for the camera scene node from yaw and pitch; roll is fixed to zero
                    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
                }
                else
                {
                    // Move the cursor to the touch position
                    Cursor* cursor = GetSubsystem<UI>()->GetCursor();
                    if (cursor && cursor->IsVisible())
                        cursor->SetPosition(state->position_);
                }
            }
        }
    }
}

void Window3DNavigator::HandleTouchBegin(StringHash /*eventType*/, VariantMap& /* eventData */)
{
    // On some platforms like Windows the presence of touch input can only be detected dynamically
    InitTouchInput();
    UnsubscribeFromEvent("TouchBegin");
}

void Window3DNavigator::HandleScreenModeChange(StringHash /*eventType*/, VariantMap& /* eventData */)
{
    if (mapVisible_)
    {
        ResetMap();
    }
}

void Window3DNavigator::ResetMap()
{
    auto* renderer = GetSubsystem<Renderer>();
    auto* graphics = GetSubsystem<Graphics>();
    auto *mapCamera = mapCameraNode_->GetComponent<Camera>();

    // reset the position of the map window based on the new screen size
    // Full screen may have changed our aspect ratio, and in the case of dual monitors
    // can even mean we are split over 2 screens giving a very wide but not so high view.
    // So work out the new ratio and work with that to try and keep our map in the UR corner,
    // and occupying more or less the same size it did before the change.

    double aspect_ratio = (double)graphics->GetWidth() / (double)graphics->GetHeight();

    // The rect used by this measures LTRB _not_ LTWH
    int left   = -1;
    int top    = graphics->GetHeight() *  8 / 100;
    int right  = graphics->GetWidth()  * 97 / 100;
    int bottom = -1;

    if (aspect_ratio >= 1024.0 / 768.0)
    {
        // Full Screen is wider compared to height than our default 1024x768 window
        // use the height to determine width

        bottom = graphics->GetHeight() * 41 / 100;
        left   = right - (int)((double)(bottom - top) * 1.3);
    }
    else
    {
        // Full Screen is taller compared to width than our default 1024x768 window
        // use the width to determine height
        left   = graphics->GetWidth()  * 64 / 100;
        bottom = top + (int)((double)(right - left) / 1.3);
    }
    // printf("Map viewscreen: [ %d %d %d %d]\n", left, top, right, bottom);

    renderer->SetViewport(1, new Viewport(context_, scene_, mapCamera,
        IntRect( left, top, right, bottom )));
}

void Window3DNavigator::HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE)
    {
        if (windowStartLocations_->IsVisible())
        {
            ShowLocationsWindow( false, false );
        }
        else if (windowMaps_->IsVisible())
        {
            ShowMapsWindow( false, false );
        }
        else
        {
            Input* input = GetSubsystem<Input>();

            if (!input->IsMouseVisible() || input->IsMouseGrabbed())
            {
                input->SetMouseMode( MM_ABSOLUTE );
                input->SetMouseVisible( true );
            }
            else
            {
                input->SetMouseMode( MM_RELATIVE );
            }
        }
    }

    if (!GetSubsystem<UI>()->GetFocusElement())
    {
        // Switch between 1st and 3rd person
        if (key == KEY_F)
        {
            firstPerson_ = !firstPerson_;
        }
        else if (key == KEY_I)
        {
            TogglePositionInfo();
        }
        // Toggle map on and off
        else if ((key == KEY_M) || (key == KEY_TAB))
        {
            ToggleMap();
        }
        // Switch between normal and noclip mode
        else if (key == KEY_F12)
        {
            ToggleNoclipMode();
        }
        // Take screenshot
        else if (key == '9')
        {
            Graphics* graphics = GetSubsystem<Graphics>();
            Image screenshot(context_);
            graphics->TakeScreenShot(screenshot);
            // Here we save in the Data folder with date and time appended
            screenshot.SavePNG(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
                Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png");
        }
        // Check for loading / saving the scene
        else if (key == KEY_F5)
        {
            File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/Window3DNavigator.xml", FILE_WRITE);
            scene_->SaveXML(saveFile);
        }
    }
}

void Window3DNavigator::HandleButtonToggleNoclip(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ToggleNoclipMode();
}

void Window3DNavigator::HandleButtonToggleMap(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ToggleMap();
}

void Window3DNavigator::HandleButtonTogglePositionInfo(StringHash /* eventType */, VariantMap& /* eventData */)
{
    TogglePositionInfo();
}

void Window3DNavigator::HandleButtonRespawn(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ShowLocationsWindow(true, true);
}

void Window3DNavigator::HandleButtonChangeMap(StringHash /* eventType */, VariantMap& /* eventData */)
{
    ShowMapsWindow(true, true);
}

void Window3DNavigator::HandleButtonSavePosition(StringHash /* eventType */, VariantMap& /* eventData */)
{
    Input* input         = GetSubsystem<Input>();

    input->SetMouseMode( MM_ABSOLUTE );
    input->SetMouseVisible( true );

    saveChanges_ = true;
    engine_->Exit();
}

void Window3DNavigator::ToggleNoclipMode(bool toggle)
{
    Button*  button = (Button *)GetSubsystem<UI>()->GetRoot()->GetChild("Noclip", true);

    Node* characterNode = scene_->GetChild("Jack", true);
    if (characterNode)
    {
        auto* body = characterNode->GetComponent<RigidBody>();

        if (toggle)
        {
            noclipActive_ = !noclipActive_;
        }

        if (noclipActive_)
        {
            body->SetUseGravity(false);
            body->SetCollisionEventMode(COLLISION_NEVER);
            body->SetCollisionMask(0x00000000);
            button->SetStyle("ToggledButton");
        }
        else
        {
            body->SetUseGravity(true);
            body->SetCollisionEventMode(COLLISION_ALWAYS);
            body->SetCollisionMask(0xffffffff);
            button->SetStyle("Button");
        }
    }
}

void Window3DNavigator::ToggleMap(bool toggle)
{
    Renderer* renderer = GetSubsystem<Renderer>();
    Button*   button   = (Button *)GetSubsystem<UI>()->GetRoot()->GetChild("Map", true);

    if (toggle)
    {
        mapVisible_ = !mapVisible_;
    }
    if (mapVisible_)
    {
        renderer->SetNumViewports(2);
        ResetMap();
        mapPositionMarker_->SetVisible(true);
        button->SetStyle("ToggledButton");
    }
    else
    {
        renderer->SetNumViewports(1);
        mapPositionMarker_->SetVisible(false);
        button->SetStyle("Button");
    }
}

void Window3DNavigator::TogglePositionInfo(bool toggle)
{
    Button* button = (Button *)GetSubsystem<UI>()->GetRoot()->GetChild("PositionInfo", true);

    if (toggle)
    {
        positionInfoVisible_ = !positionInfoVisible_;
    }
    positionText_->SetVisible( positionInfoVisible_ );

    if (positionInfoVisible_)
    {
        button->SetStyle("ToggledButton");
    }
    else
    {
        button->SetStyle("Button");
    }
}

static void bitmapDetails(SLFFile *f, bool bBitmapDirSet, struct animated_mesh *ani_ptr)
{
    if (bBitmapDirSet)
    {
        int32_t Buffer, v16, v18;

        Buffer = f->readLELong();
        v16 = f->readLELong();
        v18 = f->readLELong();

        if ((v16 > 0) && (v18 > 0))
        {
            int8_t   v12 = 0;
            uint16_t  v15;

            if (Buffer >= 3)
            {
                v12 = f->readByte();
            }
            if (Buffer >= 4)
                f->skip( 1 );

            if (v12 & 1)
                f->skip( 3 );

            v15 = f->readLEUShort();
            f->skip( v15 );
        }
    }
    else
    {
        uint32_t Buffer;

        Buffer = f->readLEULong();
        ani_ptr->num_vertices = f->readLEULong();
        ani_ptr->num_faces = f->readLEULong();

        if ((ani_ptr->num_vertices > 0) && (ani_ptr->num_faces > 0))
        {
            int8_t    v80 = 0;
            uint8_t   v81 = 0;
            uint16_t  v86;

            if (Buffer >= 3)
            {
               v80 = f->readByte();
            }
            if (Buffer >= 2)
            {
                f->skip( 40 );
            }

            if (Buffer >= 4)
            {
                v81 = f->readUByte();
                f->skip( v81* 4 );
            }
            if (v80 & 1)
            {
                v81 = f->readUByte();
                // overwrite the earlier value for the number of frames - this is the
                // one we want here
                ani_ptr->num_frames = f->readLEUShort();

                float offset = 0.0;

                if (v81 == 2)
                {
                    offset = f->readFloat();
                }

                if (v80 & 2)
                {
                    ani_ptr->frames = (vertex_t **) calloc( ani_ptr->num_frames, sizeof(vertex_t *) );

                    for (int j=0; j< ani_ptr->num_frames; j++)
                    {
                        if (ani_ptr->frames)
                        {
                            ani_ptr->frames[j] = (vertex_t *)calloc( ani_ptr->num_vertices, sizeof(vertex_t) );
                        }

                        for (int k=0; k<(int)ani_ptr->num_vertices; k++)
                        {
                            int16_t x, y, z;

                            x = f->readLEShort();
                            y = f->readLEShort();
                            z = f->readLEShort();

                            if (ani_ptr->frames && ani_ptr->frames[j])
                            {
                                ani_ptr->frames[ j ][ k ].x = ((double)x / (double)offset + ani_ptr->origin_x)  * 500.0 * kScale;
                                ani_ptr->frames[ j ][ k ].y = ((double)y / (double)offset + ani_ptr->origin_y)  * 500.0 * kScale;
                                ani_ptr->frames[ j ][ k ].z = ((double)z / (double)offset + ani_ptr->origin_z)  * 500.0 * kScale;
                                // no normals
                                // no uvs (yet)
                            }
                        }
                    }
                }
            }
            else
            {
                ani_ptr->frames = (vertex_t **) calloc( ani_ptr->num_frames, sizeof(vertex_t *) );

                // Only the first frame has vertices stored here - all the others have to be calculated
                // based on a matrix transformation

                if (ani_ptr->frames)
                {
                    ani_ptr->frames[0] = (vertex_t *)calloc( ani_ptr->num_vertices, sizeof(vertex_t) );
                }

                for (int k=0; k<(int)ani_ptr->num_vertices; k++)
                {
                    float x, y, z;

                    x = f->readFloat();
                    y = f->readFloat();
                    z = f->readFloat();

                    if (ani_ptr->frames && ani_ptr->frames[0])
                    {
                        ani_ptr->expect_matrix = true;
                        ani_ptr->frames[ 0 ][ k ].x = ((double)x + ani_ptr->origin_x) * 500.0 * kScale;
                        ani_ptr->frames[ 0 ][ k ].y = ((double)y + ani_ptr->origin_y) * 500.0 * kScale;
                        ani_ptr->frames[ 0 ][ k ].z = ((double)z + ani_ptr->origin_z) * 500.0 * kScale;
                    }
                }
                // This is xyz coords. All multiplied by 500.0 again. (Plus it looks like x coords get mucked up with [n].x = [n+1].x all way through)
            }
            // We have the same problem here that we have with the still mesh. UVs need to be assigned per vertex, when
            // they are really only meaningful for faces. So we potentially have to double vertices again - but this time
            // we have them allocated into a horrible memory array with other frames as well.
            // Fortunately with the face list at least staying constant it is at least solvable.
            ani_ptr->faces = (face_t *) malloc( ani_ptr->num_faces * sizeof(face_t) );
            if (v80 & 4)
            {
                if (ani_ptr->faces)
                {
                    for (int k=0; k<(int)ani_ptr->num_faces; k++)
                    {
                        ani_ptr->faces[k].vert[0].idx  = f->readLEUShort();
                        ani_ptr->faces[k].vert[1].idx  = f->readLEUShort();
                        ani_ptr->faces[k].vert[2].idx  = f->readLEUShort();
                        ani_ptr->faces[k].vert[0].u    = f->readFloat();
                        ani_ptr->faces[k].vert[0].v    = f->readFloat();
                        ani_ptr->faces[k].vert[1].u    = f->readFloat();
                        ani_ptr->faces[k].vert[1].v    = f->readFloat();
                        ani_ptr->faces[k].vert[2].u    = f->readFloat();
                        ani_ptr->faces[k].vert[2].v    = f->readFloat();
                        ani_ptr->faces[k].material_idx = f->readLEUShort();

                        f->skip(1);
                    }
                }
                else
                {
                    f->skip( ani_ptr->num_faces * 33 );
                }

                duplicateVerticesForUVs( ani_ptr->faces, ani_ptr->num_faces, ani_ptr->frames, &(ani_ptr->num_vertices), ani_ptr->num_frames );
            }
            else
            {
                if (ani_ptr->faces)
                {
                    for (int k=0; k<(int)ani_ptr->num_faces; k++)
                    {
                        ani_ptr->faces[k].vert[0].idx  = f->readLEULong();
                        ani_ptr->faces[k].vert[1].idx  = f->readLEULong();
                        ani_ptr->faces[k].vert[2].idx  = f->readLEULong();
                        ani_ptr->faces[k].vert[0].u    = f->readFloat();
                        ani_ptr->faces[k].vert[0].v    = f->readFloat();
                        ani_ptr->faces[k].vert[1].u    = f->readFloat();
                        ani_ptr->faces[k].vert[1].v    = f->readFloat();
                        ani_ptr->faces[k].vert[2].u    = f->readFloat();
                        ani_ptr->faces[k].vert[2].v    = f->readFloat();
                        ani_ptr->faces[k].material_idx = f->readLEULong();

                        f->skip(1);
                    }
                }
                else
                {
                    f->skip( ani_ptr->num_faces * 41 );
                }

                duplicateVerticesForUVs( ani_ptr->faces, ani_ptr->num_faces, ani_ptr->frames, &(ani_ptr->num_vertices), 1 );
            }

            ani_ptr->num_materials = f->readLEUShort();

            ani_ptr->material_buf = (uint8_t **) malloc( sizeof(uint8_t *) * ani_ptr->num_materials );
            for (int k=0; k<(int)ani_ptr->num_materials; k++)
            {
                uint8_t  type;

                type = f->readUByte();

                if (type == 0x04)
                {
                    ani_ptr->material_buf[k] = (uint8_t *)malloc(298);
                    if (298 - 1 != f->read((char *)ani_ptr->material_buf[k], 298 - 1))
                        throw SLFFileException();
                }
                else
                {
                    ani_ptr->material_buf[k] = (uint8_t *)malloc(282);
                    if (282 - 1 != f->read((char *)ani_ptr->material_buf[k], 282 - 1))
                        throw SLFFileException();
                }
            }
        }
    }
}

WizardryAnimatedMesh::WizardryAnimatedMesh(Context* context) :
    LogicComponent(context),
    num_frames_(0),
    num_vertices_(0),
    frames_(NULL),
    next_frame_(0),
    frame_delay_(1.0),
    time_counter_(0)
{
    // Only the scene update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

WizardryAnimatedMesh::~WizardryAnimatedMesh()
{
    if (frames_)
    {
        free( frames_ );
    }
}

void WizardryAnimatedMesh::Update(float timeStep)
{
    time_counter_ += timeStep;

    if (time_counter_ > frame_delay_)
    {
        time_counter_ = 0;

        auto *object = node_->GetComponent<StaticModel>();
        if (object)
        {
            auto *model  = object->GetModel();

            if (model)
            {
                Geometry *geom = model->GetGeometry(0, 0);

                if (geom)
                {
                    VertexBuffer *vb = geom->GetVertexBuffer(0);

                    if (num_vertices_ == (int)vb->GetVertexCount())
                    {
                        vb->SetData( &(frames_[ next_frame_ * num_vertices_ * 5 ]) );
                    }

                    next_frame_++;
                    if (next_frame_ == num_frames_)
                        next_frame_ = 0;
                }
            }
        }
    }
}

