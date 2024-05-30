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
 */

#ifndef WNDMAP_H
#define WNDMAP_H

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

class SLFFile;

namespace Urho3D
{
class Node;
class Scene;
}

class Avatar;
class Touch;

struct location
{
    char             name[128];
    float            x;
    float            y;
    float            z;
    float            heading;
    struct location *next;
};

struct map
{
    int              mapId;
    struct map      *next;
};

typedef struct face   face_t;

class WizardryMaterial
{
public:
    WizardryMaterial(Urho3D::String hash) { hash_ = hash; isWater_ = false; currentTextureIdx_ = 0; mapsTo_ = -1;}

    void SetMaterial(Urho3D::SharedPtr<Urho3D::Material> material) { material_ = material; }
    bool IsWater()      { return isWater_;      }
    bool IsCollidable() { return isCollidable_; }

    Urho3D::SharedPtr<Urho3D::Material> material_;

    Urho3D::Vector<Urho3D::String> textureNames_;

    Urho3D::String  hash_;
    bool            isCollidable_;
    bool            isWater_;
    float           y_;
    int             currentTextureIdx_;
    float           textureUpdateRate_;
    float           textureUpdateCounter_;
    int             mapsTo_;
};

class WizardryAnimatedMesh : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(WizardryAnimatedMesh, Urho3D::LogicComponent);

public:
    explicit WizardryAnimatedMesh(Urho3D::Context* context);

    ~WizardryAnimatedMesh();

    void Update(float timeStep) override;

public:
    int           num_frames_;
    int           num_vertices_;
    float        *frames_;
    int           next_frame_;
    float         frame_delay_;
    float         time_counter_;
};

class Window3DNavigator : public Urho3D::Object
{
    URHO3D_OBJECT(Window3DNavigator, Urho3D::Object)

public:
    Window3DNavigator( int map_id, float x, float y, float z, float heading, bool isSavePosition, int32_t *visitedMaps );
    ~Window3DNavigator();

    int  exec();

    enum DialogCode { Rejected, Accepted, ChangeMap };

    void AddTgaToResourceCache( Urho3D::String folder, Urho3D::String slf, Urho3D::String texture_name );
    void AddTgaToResourceCache( Urho3D::String texture_name );

    int   getMapId()   { return mapId_;  }
    void  getPosition(float *x, float *y, float *z);
    float getHeading();

private:
    // Use Urho3D naming convention for Urho3D member variables

    /// The handle to the graphics engine itself
    //  - it seems wasteful performance-wise to be starting and stopping
    //    engines like this every time we open or close a Window using
    //    Urho3D but it does seem to be the intent of the API to us it
    //    that way.
    Urho3D::SharedPtr<Urho3D::Engine>                     engine_;
    /// Scene.
    Urho3D::SharedPtr<Urho3D::Scene>                      scene_;
    /// Camera scene node.
    Urho3D::SharedPtr<Urho3D::Node>                       cameraNode_;

private:
    void ScaleForHiDPI();
    void NewMap( int map_id );
    int  LoadPVL( int map_id );
    int  LoadMaterial( Urho3D::String foldername, uint8_t *material_buf );
    void LoadMesh( SLFFile *f );
    void LoadLights( SLFFile *f );
    void LoadTriggers( SLFFile *f );
    void SkipMonsters( SLFFile *f );
    void SkipItems( SLFFile *f );
    void SkipPropsOrBitmaps( SLFFile *f );
    void SkipCameras( SLFFile *f );
    void SkipFogOptions( SLFFile *f );

    void processProperties( SLFFile *f, bool expect_matrix, void *v );
    void processXRefs( SLFFile *f, char *trigger_name_OUT, float *xyz_OUT );

    bool isTGAFile32Bit( Urho3D::String tga_file );

    void UpdateAnimatedMeshes(float timeStep);
    void UpdateAnimatedTextures(float timeStep);

    void AddWaterMaterialPlaneAt(float y, Urho3D::SharedPtr<Urho3D::Material> m);

    Urho3D::SharedPtr<Urho3D::Material>  FindWaterMaterialPlaneAt(float y);
    Urho3D::SharedPtr<Urho3D::Texture2D> CreateReflectionTextureAtPlane(float y);

    Urho3D::Vector<int32_t> CollateMaterials(face_t *faces, int num_faces);


    /// Create static scene content.
    void CreateScene(int mapId);
    /// Create the toolbar
    void CreateUI();
    /// Create central window listing all available maps
    void CreateMapsWindow();
    /// Create central windows listing known starting positions
    void CreateArrivalPointWindow();
    /// Create controllable character.
    void CreateAvatar();
    /// Reposition the character
    void SetAvatarPosition(const Urho3D::Vector3 &position, const float heading);
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // Turn map display on or off
    void ToggleMap(bool toggle=true);
    // Turn position information on or off
    void TogglePositionInfo(bool toggle=true);
    // Turn on or off gravity and object collision framework
    void ToggleNoclipMode(bool toggle=true);

    Urho3D::Vector<WizardryMaterial *> waterPlanes_;
    Urho3D::Vector<WizardryMaterial *> materials_;
    Urho3D::Vector<void *> stuffToFreeLater_;

    /// Touch utility object.
    Urho3D::SharedPtr<Touch> touch_;
    /// The controllable character component.
    Urho3D::WeakPtr<Avatar> character_;
    /// First person camera flag.
    bool firstPerson_;

    Urho3D::SharedPtr<Urho3D::Text>    positionText_;

    Urho3D::SharedPtr<Urho3D::Node>    mapCameraNode_;
    Urho3D::SharedPtr<Urho3D::Sprite>  mapPositionMarker_;

    Urho3D::Vector3            worldMin_;
    Urho3D::Vector3            worldMax_;

    Urho3D::SharedPtr<Urho3D::Window>  windowStartLocations_;
    Urho3D::SharedPtr<Urho3D::Window>  windowMaps_;
    struct location                   *arrivalPts_;
    struct map                        *maps_;

protected:
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;
    /// Flag to indicate whether touch input has been enabled.
    bool touchEnabled_;
    /// Mouse mode option to use in the sample.
    //MouseMode useMouseMode_;

private:
    void ResetMap();
    void InitTouchInput();
    void HandleScreenModeChange(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& eventData);
    void HandleKeyUp(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& eventData);
    void HandleKeyDown(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& eventData);
    void HandleSceneUpdate(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& eventData);
    void HandleTouchBegin(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& eventData);
    void HandleLocationsClosePressed(Urho3D::StringHash /* eventType */, Urho3D::VariantMap& /* eventData */);
    void HandleMapsClosePressed(Urho3D::StringHash /* eventType */, Urho3D::VariantMap& /* eventData */);
    void HandleLocationChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMapChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);


    void HandleButtonToggleNoclip(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);
    void HandleButtonToggleMap(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);
    void HandleButtonTogglePositionInfo(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);
    void HandleButtonRespawn(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);
    void HandleButtonChangeMap(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);
    void HandleButtonSavePosition(Urho3D::StringHash /*eventType*/, Urho3D::VariantMap& /*eventData*/);

    void ShowLocationsWindow( bool show, bool lock_mouse );
    void ShowMapsWindow( bool show, bool lock_mouse );
    void ShowDialog( bool show, bool lock_mouse );

    double hiDpiScale_;

    /// Screen joystick index for navigational controls (mobile platforms only).
    unsigned screenJoystickIndex_;
    /// Screen joystick index for settings (mobile platforms only).
    unsigned screenJoystickSettingsIndex_;
    /// map shown in navigator or not
    bool mapVisible_;
    /// position co-ordinates shown on screen or not
    bool positionInfoVisible_;
    /// Whether gravity is turned off and we can walk through walls etc.
    bool noclipActive_;
    /// true if this window was opened to change the current save position (as opposed to a portal)
    /// Save positions can only be altered to already visited maps
    bool isSavePosition_;
    /// List of map ids already visited (for isSavePosition_). Terminated by -1.
    int32_t *visitedMaps_;
    /// Id of the map loaded
    int mapId_;

    Urho3D::String pvlFilename_;

    bool   newMap_;
    bool   saveChanges_;
    float  heading_;
};

#endif
