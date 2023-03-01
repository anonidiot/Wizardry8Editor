/*
 * Copyright (C) 2022-2023 Anonymous Idiot
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

// NB: This is a mishmash of several of the Urho3D examples to create an
//     interesting unofficial About Urho3D dialog box. The samples have
//     been copied and pasted whole at the end of the file - because we
//     selectively need to disable some of the methods, which isn't so
//     easy to accomplish when they are #include'd instead. MIT License
//     on those portions is still intact.

#include <QTimer>
#include <QMouseEvent>

#include "DialogAboutUrho3D.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Physics2D/CollisionBox2D.h>
#include <Urho3D/Physics2D/CollisionCircle2D.h>
#include <Urho3D/Physics2D/PhysicsEvents2D.h>
#include <Urho3D/Physics2D/PhysicsWorld2D.h>
#include <Urho3D/Physics2D/RigidBody2D.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/ThirdParty/SDL/SDL_events.h>
#include <Urho3D/ThirdParty/SDL/SDL_mouse.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/FontFace.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Urho2D/ParticleEffect2D.h>
#include <Urho3D/Urho2D/ParticleEmitter2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

#define EPSILON   1e-9

// For URHO3D_LOGINFO() macro
#define URHO3D_LOGGING 1
#include <Urho3D/IO/Log.h>

static const int kLowDpiWidth  = 800;
static const int kLowDpiHeight = 600;

using namespace Urho3D;

// The contents of the Urho3D/LICENSE file as a Null terminated C-string
static const char urho3d_license[] =
{
#include "urho3d_license.xxd"
, 0
};

static unsigned char urho3d_authors[] = // not const - we have to modify it to chunk it into pages
{
#include "urho3d_authors.xxd"
, 0
};

static const char *btTexturePaths[] =
{
    "Textures/BlowtorchBlueFlame.png",
    "Textures/BlowtorchOrange1.png",
    "Textures/BlowtorchOrange2.png",
    "Textures/BlowtorchOrange3.png",
    "Textures/BlowtorchOrange4.png"
};

#define MAGIC_NUMBER_FIX_COLLISIONS    2
#define MAGIC_NUMBER_FIX_TEXT_SCALE    1.3
#define MAGIC_NUMBER_FLAME_SIZE        2.0

DialogAboutUrho3D::DialogAboutUrho3D(QWidget *parent)
    : QDialog(parent),
    Object(new Urho3D::Context()),
    blowtorchFrame_(1),
    m_drawTextTimer(0)
{
    resize( kLowDpiWidth, kLowDpiHeight );
    show();

    engine_ = new Urho3D::Engine(context_);

    VariantMap engineParameters;

    engineParameters[ EP_FRAME_LIMITER   ] = false;
    engineParameters[ EP_EXTERNAL_WINDOW ] = (void*)(this->winId());
    engineParameters[ EP_FULL_SCREEN     ] = false;
    engineParameters[ EP_HEADLESS        ] = false;
    engineParameters[ EP_SOUND           ] = false;
    engineParameters[ EP_RESOURCE_PATHS  ] = ":/CoreData.pak";

    // Doesn't produce useful results on Windows in either OpenGL or DirectX
    // so leaving Qt in control of window and working it out ourselves.
    // engineParameters[ EP_HIGH_DPI        ] = true;


    if (engine_->Initialize( engineParameters ))
    {
        Graphics* graphics = GetSubsystem<Graphics>();

        int pixelsWidth  = 0;
        int pixelsHeight = 0;

        window_ = graphics->GetWindow();

        SDL_GetWindowSize(window_, &pixelsWidth, &pixelsHeight);
        hiDpiScale_ = (double)pixelsWidth / (double)kLowDpiWidth;

        // A mishmash of the elements of the 2 samples as well
        // as the logo from the core Samples repositioned to centre top
        // and the MIT license and readme quoted
        CreateLogo();
        CreateScene();

        CreateGroundLevel();
        SetupViewport();

        CreateTextPages();
        m_currentPage = 0;
        CreateText( m_pages[m_currentPage] );

        CreateSprites();
        for (int k=0;k < (int)sprites_.Size(); k++)
        {
            // Change the colors of all the fish so instead of being all light
            // colours they are dark colours (to contrast with the license we are
            // drawing over the top
            sprites_[k]->SetColor(Color(Random(0.5f), Random(0.5f), Random(0.5f)));
        }

        // Hook up to the frame update events
        SubscribeToEvent(E_UPDATE,    URHO3D_HANDLER(DialogAboutUrho3D, HandleUpdateWrap));
        SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(DialogAboutUrho3D, HandleMouseMoveWrap ));
        SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(DialogAboutUrho3D, HandleMouseClick));

        // Load XML file containing default UI style sheet
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile*       style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        Font*          font  = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

        // Set the loaded style as default style
        SharedPtr<UIElement> uiRoot( GetSubsystem<UI>()->GetRoot() );
        uiRoot->SetDefaultStyle(style);

        // Create the Ok Button
        Button* button = uiRoot->CreateChild<Button>();
        button->SetStyleAuto();
        button->SetSize((int)(75.0 * hiDpiScale_), (int)(30.0 * hiDpiScale_));
        button->SetPosition((int)(660.0 * hiDpiScale_), (int)(540.0 * hiDpiScale_));

        Text* buttonText = button->CreateChild<Text>();
        buttonText->SetAlignment(HA_CENTER, VA_CENTER);
        buttonText->SetFont(font, (int)(12.0 * hiDpiScale_));
        buttonText->SetText("Ok");

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(DialogAboutUrho3D, HandleClose));

        // Create a Page down button
        button = uiRoot->CreateChild<Button>();
        button->SetStyleAuto();
        button->SetSize((int)(100.0 * hiDpiScale_), (int)(30.0 * hiDpiScale_));
        button->SetPosition((int)(535.0 * hiDpiScale_), (int)(540.0 * hiDpiScale_));

        buttonText = button->CreateChild<Text>();
        buttonText->SetAlignment(HA_CENTER, VA_CENTER);
        buttonText->SetFont(font, (int)(12.0 * hiDpiScale_));
        buttonText->SetText("Next Page");

        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(DialogAboutUrho3D, HandlePageDown));

        // Subscribe to notifications of when letters collide
        SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(DialogAboutUrho3D, HandleLetterCollision));

        Input* input = GetSubsystem<Input>();
        input->SetMouseVisible(true);
        input->SetMouseMode(MM_FREE);

        // We misuse the different types of cursors here to do an animation; it
        // turns out that calling UI->SetCursor() with a different cursor in the
        // update handler doesn't work very well at all - we can't do it fast enough,
        // and it is stopping after the first sequence. So instead we set various
        // cursor shapes on the same cursor to different frames of the animation,
        // and just change the shape within the update notification. This has the
        // unfortunate consequence that we HAVE to call the SetShape() function on
        // every single update, though, as the lib itself is going to try to reset
        // to CS_NORMAL constantly otherwise. So even if we want to animate at a
        // slower rate than that, we still have to refresh with the current frame.
        blowtorch_ = new Cursor(context_);
        for (int k=0; k < (int)(sizeof(btTexturePaths) / sizeof(const char *)); k++)
        {
            Texture2D*         blowtorchTex = cache->GetResource<Texture2D>( btTexturePaths[k] );
            SharedPtr<Image>   blowtorchImg = blowtorchTex->GetImage();
            IntRect            r( 0, 0, blowtorchTex->GetWidth(), blowtorchTex->GetHeight() );
            IntVector2         v(  blowtorchTex->GetWidth() / 2, blowtorchTex->GetHeight() / 2 );

            // The + k  in here is where we misuse the cursor types
            blowtorch_->DefineShape( (CursorShape)(CS_NORMAL + k), blowtorchImg, r, v );
        }
        GetSubsystem<UI>()->SetCursor(blowtorch_);

        // SDL uses a different implementation module on Windows than on Linux for
        // its mouse functionality. Net result is that the mouse tracks properly in
        // an Urho3d external window on Windows, but not on Linux. We have to map it
        // through if we are on Linux, by capturing the mouse movements and clicks;
        // we definitely do not want to do this on Windows where doing so doesn't just
        // mess up mouse navigation but also causes a segfault on dialog close.

#if !defined(WIN32) && !defined(WIN64)
        setMouseTracking(true);
#endif
        // rig the scene to a QTimer to update the rendering every time it times out;
        // Urho3D isn't driving the main thread to handle this itself when we run under Qt
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
        m_timer.start(16);
    }
}

DialogAboutUrho3D::~DialogAboutUrho3D()
{
    // Urrho3D uses SharedPtrs for just about everything
    // so we should NOT be releasing the various objects
    // instantiated with new() in this class. This includes
    // the sprites_ Vector and the engine_ itself

    // This doesn't actually unregister our cursor - it should, based on the code
    // in the UI::SetCursor() class, but it doesn't. A reference is retained
    // in SDL and it continues to be used. Can get segfaults if we try and free
    // the pointer too.
    GetSubsystem<UI>()->SetCursor(NULL);

    // There's no way to reset the cursors to the original operating system ones
    // other than to reinit the SDL_mouse; but because that's platform dependent
    // we have to come back a few levels and reinit SDL_video itself.
    // Yes it's stupid doing an init in a destructor, but it's better than forcing
    // every navigator to have to redo it, and we created the mess in this class,
    // so we should fix it.
    // This has to be done before the engine exits or it doesn't work.
    SDL_VideoInit(0);

#if !defined(WIN32) && !defined(WIN64)
    setMouseTracking(false);
#endif
}

void DialogAboutUrho3D::closeEvent(QCloseEvent *e)
{
    (void) e;

    engine_->Exit();
    deleteLater();
}

void DialogAboutUrho3D::OnTimeout()
{
    // render the frame, or gracefully close down this dialog and its context
    if (engine_ && !engine_->IsExiting())
        engine_->RunFrame();
    else
    {
        close();
        deleteLater();
    }
}

void DialogAboutUrho3D::CreateTextPages()
{
    m_pages[0] = (char *)urho3d_authors + 3;

    int x = 0;
    int y = 0;
    int n = 0;

    for (char *p = (char *)urho3d_authors+3; *p; p++)
    {
        if (x >= 76)
        {
            // Manual word-wrap. The Text node supports wordwrap, but the automatic
            // layout is overriding any attempt I make to set a size on it, and it
            // won't work without a known width, so we wrap the source text here
            // manually. This isn't as nice because the font could be proportional
            // but as Anonymous Pro isn't we can get away with it.

            char *q;
            // look backwards for last space
            for (q = p; (q > (char *)urho3d_authors) && (*q != '\n') && (*q != ' '); q--)
                ;
            // if it's a newline, give up, text is going to overflow because no spaces
            // on the line. A number of URLS are in the text and some of these are long
            // enough to do that. I can't break them anywhere without adding additional
            // characters, and just trying to keep this simple.
            if (*q != '\n')
            {
                p = q;
                // Change it to a new line
                *p = '\n';
            }
        }
        // NOT else if
        if (*p == '\n')
        {
            y++;
            x = 0;
            if (y > 20)
            {
                *p = 0;
                m_pages[++n] = p+1;
                y = 0;
                x = 0;
            }
        }
        else
        {
            x++;
        }
    }
    m_pages[++n] = urho3d_license;
    m_numPages   = n+1;

    // We should now have m_numPages pages of null terminated text each containing
    // 20/21 lines and any lines longer than 76 chars should have been manually
    // wordwrapped if possible
}

void DialogAboutUrho3D::CreateGroundLevel()
{
    scene_->CreateComponent<PhysicsWorld2D>();

    // Create ground.
    Node* groundNode = scene_->CreateChild("Ground");
    groundNode->SetPosition(Vector3(0.0f, -3.0f, 0.0f));
    groundNode->SetScale(Vector3(200.0f, 1.0f, 0.0f));

    // Create 2D rigid body for gound
    groundNode->CreateComponent<RigidBody2D>();

    // Create box collider for ground
    CollisionBox2D* groundShape = groundNode->CreateComponent<CollisionBox2D>();
    // Set box size
    groundShape->SetSize(Vector2(0.32f, 0.32f));
    // Set friction
    groundShape->SetFriction(0.5f);
}

void DialogAboutUrho3D::HandleClose(StringHash /* eventType */, VariantMap& /* eventData */)
{
    engine_->Exit();
    deleteLater();
}

void DialogAboutUrho3D::HandlePageDown(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;

    // Delete all the text islands
    PODVector<Node*> letterNode;

    scene_->GetChildrenWithComponent<StaticSprite2D>( letterNode );

    for (int k=0; k < (int)letterNode.Size(); k++)
    {
        scene_->RemoveChild( letterNode[k] );
    }
    letterNode.Clear();

    // Now start again with the next page

    ++m_currentPage;
    if (m_currentPage >= m_numPages)
        m_currentPage = 0;
    CreateText( m_pages[m_currentPage] );
}

void DialogAboutUrho3D::HandleUpdateWrap(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    if (m_drawTextTimer)
    {
        m_drawTextTimer -= timeStep;
        if (m_drawTextTimer <= 0)
        {
            m_drawTextTimer = 0;
            HandlePageDown( eventType, eventData );
        }
    }

    HandleUpdate(eventType, eventData);
}

void DialogAboutUrho3D::HandleMouseClick(StringHash eventType, VariantMap& eventData)
{
    UI       *ui    = GetSubsystem<UI>();
    Input    *input = GetSubsystem<Input>();

    // only fall through to the blowtorch routine if the user isn't interacting with
    // a real UI element, eg. button

    WeakPtr<UIElement> element(ui->GetElementAt(input->GetMousePosition(), true));

    if (!element)
    {
        HandleMouseMoveWrap(eventType, eventData);
    }
}

void DialogAboutUrho3D::HandleMouseMoveWrap(StringHash eventType, VariantMap& eventData)
{
    Input              *input           = GetSubsystem<Input>();
    ParticleEmitter2D  *particleEmitter = particleNode_->GetComponent<ParticleEmitter2D>();

    PhysicsWorld2D     *physics         = scene_->GetComponent<PhysicsWorld2D>();

    if (input->GetMouseButtonDown(MOUSEB_LEFT))
    {
        particleEmitter->SetEmitting( true );

        if (blowtorchFrame_ != 0)
        {
            blowtorchFrame_ = 0;
            blowtorch_->SetShape( CS_NORMAL );
        }

        // Call into the Particle demo's default mouse handler
        HandleMouseMove( eventType, eventData );

        // re-enable the physics engine if it is off - we had this initially off
        // to avoid doing unnecessary calculations on a static scene
        physics->SetUpdateEnabled(true);

        // Change the colour of the letters under the flame:
        ColouriseLetters( input->GetMousePosition().x_, input->GetMousePosition().y_ );

        // See if there's a letter right under the mouse, and if there is set it falling
        RigidBody2D* rigidBody = physics->GetRigidBody(input->GetMousePosition().x_, input->GetMousePosition().y_);
        if (rigidBody)
        {
            Node *pickedNode = rigidBody->GetNode();

            StaticSprite2D* staticSprite = pickedNode->GetComponent<StaticSprite2D>();
            if (staticSprite)
            {
                // If we couldn't find a static sprite then the rigid body we've picked up on
                // is probably the ground plane. Don't give this a colour or change it to dynamic
                staticSprite->SetColor( Color(0.2f, 0.2f, 0.2f, 0.0f) /* Dark grey */ );

                // We change the letter we're over to dynamic, it will begin falling due to gravity
                // straight away. And the collision notify function we've registerd, HandleLetterCollisions()
                // is going to switch on dynamic on everything it hits on the way down, and everything
                // they in turn hit etc. So we should get a cascade effect.
                if (rigidBody->GetBodyType() == BT_STATIC)
                {
                    m_fallenLetters++;
                    rigidBody->SetBodyType(BT_DYNAMIC);
                }
            }
        }
    }
    else
    {
        particleEmitter->SetEmitting( false );
    }

    // Critical mass - will be triggered by mouse movement if appropriate too
    if (m_fallenLetters > m_totalLetters * 60 / 100)
    {
        // Set all the letters to dynamic so they'll fall
        PODVector<Node*> spriteNodes;

        // We search for sprite nodes rather than rigid bodies to filter out the ground node
        scene_->GetChildrenWithComponent<StaticSprite2D>( spriteNodes );

        for (int k=0; k < (int)spriteNodes.Size(); k++)
        {
            RigidBody2D* body = spriteNodes[k]->GetComponent<RigidBody2D>();

            if (body->GetBodyType() == BT_STATIC)
            {
                m_fallenLetters++;
                body->SetBodyType(BT_DYNAMIC);
            }
        }
        // Start a timer to draw the next text
        m_drawTextTimer = 50;
    }
}

void DialogAboutUrho3D::HandleLetterCollision(StringHash /* eventType */, VariantMap& eventData)
{
    Node* nodeA = static_cast<Node*>(eventData[PhysicsBeginContact2D::P_NODEA].GetPtr());
    Node* nodeB = static_cast<Node*>(eventData[PhysicsBeginContact2D::P_NODEB].GetPtr());

    Node* groundNode = scene_->GetChild("Ground");

    // Not interested in ground based collisions
    if ((nodeA != groundNode) && (nodeB != groundNode))
    {
        RigidBody2D* bodyA = nodeA->GetComponent<RigidBody2D>();
        RigidBody2D* bodyB = nodeB->GetComponent<RigidBody2D>();

        if (bodyA->GetBodyType() == BT_STATIC)
        {
            m_fallenLetters++;
            bodyA->SetBodyType(BT_DYNAMIC);
        }
        if (bodyB->GetBodyType() == BT_STATIC)
        {
            m_fallenLetters++;
            bodyB->SetBodyType(BT_DYNAMIC);
        }
    }
}

#if !defined(WIN32) && !defined(WIN64)
// expose an unlisted methods so that we can pass through a mouse click to SDL
extern "C" { int SDL_SendMouseButton(void *window, int mouseID, Uint8 state, Uint8 button); }

void DialogAboutUrho3D::mouseMoveEvent(QMouseEvent *event)
{
    (void)event;
    QPoint mousePosition = this->mapFromGlobal(QCursor::pos());

    if (engine_ && !engine_->IsExiting())
        SDL_WarpMouseInWindow( window_, mousePosition.x(), mousePosition.y() );
}

void DialogAboutUrho3D::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint mousePosition = this->mapFromGlobal(QCursor::pos());

        if (engine_ && !engine_->IsExiting())
        {
            SDL_WarpMouseInWindow( window_, mousePosition.x(), mousePosition.y() );
            SDL_SendMouseButton( window_, 0, SDL_PRESSED, SDL_BUTTON_LEFT);
        }
    }
}

void DialogAboutUrho3D::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint mousePosition = this->mapFromGlobal(QCursor::pos());

        if (engine_ && !engine_->IsExiting())
        {
            SDL_WarpMouseInWindow( window_, mousePosition.x(), mousePosition.y() );
            SDL_SendMouseButton( window_, 0, SDL_RELEASED, SDL_BUTTON_LEFT);
        }
    }
}
#endif

void DialogAboutUrho3D::CreateText( const char *text )
{
    ResourceCache* cache    = GetSubsystem<ResourceCache>();
    UI*            ui       = GetSubsystem<UI>();
    Graphics*      graphics = GetSubsystem<Graphics>();
    Renderer*      renderer = GetSubsystem<Renderer>();

    // Construct new Text object, set string to display and font to use
    currentText_ = ui->GetRoot()->CreateChild<Text>();
    currentText_->SetText( text );
    currentText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), (int)(12.0 * hiDpiScale_));
    currentText_->SetColor(Color(1.0, 1.0, 1.0));

    // Position the text relative to the screen center
    currentText_->SetHorizontalAlignment(HA_CENTER);
    currentText_->SetVerticalAlignment(VA_CENTER);
    currentText_->SetPosition(0, ui->GetRoot()->GetHeight() / 15);

    // We can stop here and have text display normally
    // The code that follows chops out each individual letter in
    // the text just rendered and turns it into a physical object
    // so we can have them come crashing to the bottom of the screen

    m_totalLetters  = 0;
    m_fallenLetters = 0;

    // First up correct the aspect ratio of the camera - during scene setup this is set to 1.0
    // default instead of width / height, and Urho3D doesn't correct it until it starts running
    // renderer updates.
    // We have to correct now ourselves, or none of the ScreenToWorldPoint() calls below will be correct
    cameraNode_->GetComponent<Camera>()->SetAspectRatio( (float)graphics->GetWidth() / (float)graphics->GetHeight() );

    // Next disable the physics engine before we start putting any of these text chars
    // on screen, or nobody will be able to read it before they start drifting out of
    // position. We'll re-enable it when the user first starts using their flame on the
    // text. This should also avoid wasting CPU on unwanted physics calculations in the
    // meantime.
    PhysicsWorld2D *physics = scene_->GetComponent<PhysicsWorld2D>();
    physics->SetUpdateEnabled(false);

    // We need to use the unscaled font size here, or else the text is too big
    // Think it's because of an explict scale set on the RigidBody node below.
    FontFace* face = currentText_->GetFont()->GetFace( 12 );

    IntVector2 textLoc = currentText_->GetScreenPosition();

    // Retrieve an alpha only pixmap copy of the font (can be multiple pixmaps) and
    // convert it/them to regular RGB textures that can be piecemeal targeted with U and V
    // co-ordinates to display individual characters
    // We keep the alpha data around so simplify locating the centre and size of the glyphs,
    // since it is 1/4 the size of the RGBA texture we're creating
    Vector<SharedPtr<Texture2D> >           textures( Vector<SharedPtr<Texture2D> >( face->GetTextures() ) );
    Vector<SharedArrayPtr<unsigned char> >  alphaData;

    for (int k = 0; k < (int)textures.Size(); k++)
    {
        if (textures[ k ]->GetFormat() == Graphics::GetAlphaFormat())
        {
            int   width  = textures[ k ]->GetWidth();
            int   height = textures[ k ]->GetHeight();

            int alphaTextureDataSize = width * height * textures[ k ]->GetComponents();
            SharedArrayPtr<unsigned char> alphaTextureData(new unsigned char[alphaTextureDataSize]);
            textures[ k ]->GetData( 0, alphaTextureData.Get() );
            alphaData.Push( alphaTextureData );

            SharedPtr<Image> alpha( new Image( context_ ) );
            alpha->SetSize( width, height, textures[ k ]->GetComponents() );
            alpha->SetData( alphaTextureData.Get() );

            SharedPtr<Image> rgbText = alpha->ConvertToRGBA();

            SharedPtr<Texture2D> rgbaTexture(new Texture2D(context_));
            rgbaTexture->SetMipsToSkip(QUALITY_LOW, 0);
            rgbaTexture->SetNumLevels(1);
            rgbaTexture->SetSize( width, height, Graphics::GetRGBAFormat() );
            rgbaTexture->SetData(0, 0, 0, width, height, rgbText->GetData());

            textures[ k ] = rgbaTexture;
        }
    }

    // Proceed through the entire text array creating objects for each non-blank character

    // I had issues trying to use currentText_->GetText()[] as input to GetGlyph() - it couldn't
    // find anything with a diacritical mark; so we're manually parsing UTF8 off the original
    // string as a workaround here.
    int text_len = currentText_->GetText().Length();
    const char *s = text;
    for (int k = 0; k < text_len; k++)
    {
        unsigned chr = String::DecodeUTF8(s); // s is incremented as a result of this call

        // With UTF8 input apparently the Urho3D string Length() function can include the null
        // terminator (and beyond) when actual unicode characters are present in the string
        // because it counts each byte instead of each character when working with a variable
        // size format like it. We manually break here if we get this because we setup issues
        // in the physics engine if we attempt to setup a collision shape on a zero size object.
        if (chr == '\0')
            break;

        if ((chr != ' ') && (chr != '\n'))
        {
            const FontGlyph *glyph = face->GetGlyph( chr );

            if (glyph && (glyph->page_ < textures.Size()))
            {
                // We've got multiple co-ordinate systems and multiple different origin points to
                // navigate here. Convinced I've got some bugs in here due to the reliance on some
                // magic numbers, but it's mostly ok.

                // pos is the position of the character relative to the text block it is part of
                Vector2 pos = currentText_->GetCharPosition( k );
                //  Anonymous Pro is a non proportional font with all chars same size, but most aren't
                Vector2 sz  = currentText_->GetCharSize( k );

                // Add in the text block's location to turn this into absolute screen position
                pos += Vector2( textLoc );
                // Now account for any white space in the glyph itself due to it not ascending
                // to maximum height or having leading white space
                pos += Vector2( (int)(glyph->offsetX_ * hiDpiScale_), (int)(glyph->offsetY_ * hiDpiScale_) );

                // Convert the 2D window co-ordinate to a space in 3D world co-ordinates that 3D
                // objects can be placed at (even though we're still staying 2D)
                // The 10.0f z co-ordinate comes from the fact the camera is at -10z - we inherited
                // the camera as the samples chose to set it up. It should be a #define or const but
                // I don't want to edit their original sources below.
                Vector3 worldPos = renderer->GetViewport(0)->ScreenToWorldPoint( pos.x_, pos.y_, 10.0f );
                Vector3 worldSz  = renderer->GetViewport(0)->ScreenToWorldPoint( pos.x_ + sz.x_, pos.y_ + sz.y_, 10.0f );

//printf("%f %f -> %f %f  %f %f ->", pos.x_, pos.y_, worldPos.x_, worldPos.y_, sz.x_, sz.y_);
                // Reset the sz vector based on actual world size now
                sz.x_ = fabs( worldSz.x_ - worldPos.x_ );
                sz.y_ = fabs( worldSz.y_ - worldPos.y_ );
//printf("%f %f '%c'\n", sz.x_, sz.y_, text[k]);

                Node* node  = scene_->CreateChild("RigidBody");
                node->SetPosition( worldPos );
                // None of the Urho samples make it at all clear how the scaling is supposed to work
                // with 2D objects. What is the base size we're actually scaling and where does it even
                // come from? All their examples are using hard-coded floats without explaining what
                // went into choosing them. I just had to do the same till I got something that looked right.
                node->SetScale( MAGIC_NUMBER_FIX_TEXT_SCALE );

                m_totalLetters++;

                // Create rigid body
                RigidBody2D* body = node->CreateComponent<RigidBody2D>();
                // Start the bodies out as static, we'll turn them to dynamic as they get hit or
                // clicked on
                body->SetBodyType(BT_STATIC);

                // Create a new sprite, set it to use the texture area appropriate for the
                // current letter
                SharedPtr<Sprite2D> sprite(new Sprite2D(context_));
                sprite->SetTexture( textures[glyph->page_] );
                sprite->SetRectangle( IntRect( glyph->x_,
                                               glyph->y_,
                                               glyph->x_ + glyph->texWidth_,
                                               glyph->y_ + glyph->texHeight_ ) );

                // Our reference point is the top left hand corner of glyph; this
                // fixes the text baseline rendering
                sprite->SetHotSpot(Vector2(0.0, 1.0));

                StaticSprite2D* staticSprite = node->CreateComponent<StaticSprite2D>();
                staticSprite->SetSprite( sprite );
                staticSprite->SetColor( Color::WHITE );

                // Accurately modelling the letters falling requires using polygons, but
                // this is unrewarding - it's too much for the Physics engine dealing with
                // that many polygons on screen with the number of vertices we need. The
                // rendering stalls. So simplifying with just a Circle or a Box centred
                // at the appropriate place depending on its letter shape.

                // Find edges so we can determine centre and size
                int min_x = 1000000;
                int max_x = 0;
                int min_y = 1000000;
                int max_y = 0;
                for (int x=0; x < glyph->texWidth_; x++)
                {
                    for (int y=0; y < glyph->texHeight_; y++)
                    {
                        if (alphaData[glyph->page_].Get()[ (glyph->y_ + y) * textures[glyph->page_]->GetWidth() +
                                                           (glyph->x_ + x) ])
                        {
                            if (min_x > x)
                                min_x = x;
                            if (max_x < x)
                                max_x = x;
                            if (min_y > y)
                                min_y = y;
                            if (max_y < y)
                                max_y = y;
                        }
                    }
                }
                Vector2 centre;

                // The +1 is to avoid situation where max and min are the same, which
                // messes up the physics
                float min_x_f = (float)min_x     / (float)glyph->texWidth_ * sz.x_;
                float max_x_f = (float)(max_x+1) / (float)glyph->texWidth_ * sz.x_;
                float min_y_f = (float)min_y     / (float)glyph->texHeight_ * sz.y_;
                float max_y_f = (float)(max_y+1) / (float)glyph->texHeight_ * sz.y_;

                centre.x_ = min_x_f + (max_x_f - min_x_f)/2;
                centre.y_ = min_y_f + (max_y_f - min_y_f)/2;

                switch (chr)
                {
                    // treat as round
                    case 'C':
                    case 'D':
                    case 'G':
                    case 'J':
                    case 'O':
                    case 'Q':
                    case 'S':
                    case 'U':
                    case 'a':
                    case 'c':
                    case 'e':
                    case 'j':
                    case 'o':
                    case 's':
                    case 'u':
                    case '6':
                    case '9':
                    case '0':
                    case '(':
                    case ')':
                    {
                        auto* ball = node->CreateComponent<CollisionCircle2D>();

                        ball->SetCenter( centre );
                        // Without the second divide by 2 it doesn't look right - objects stop before they
                        // touch each other. Getting a bit sick of these magic numbers that don't make sense.
                        // FIXME: Got to be a bug in the calculations somewhere
                        ball->SetRadius( (max_y_f - min_y_f) / 2 / MAGIC_NUMBER_FIX_COLLISIONS);
                        ball->SetDensity(1.0f);
                        ball->SetFriction(0.5f);
                        ball->SetRestitution(0.1f);
                        break;
                    }

                    // treat as square
                    default:
                    {
                        auto* box = node->CreateComponent<CollisionBox2D>();

                        box->SetCenter( centre );
                        // Same problem as the ball, needs an additional adjustment to stop objects from
                        // stopping before they hit one another.
                        box->SetSize( (max_x_f - min_x_f)/2, (max_y_f - min_y_f) / MAGIC_NUMBER_FIX_COLLISIONS );
                        box->SetDensity(1.0f);
                        box->SetFriction(0.5f);
                        box->SetRestitution(0.1f);
                        break;
                    }
                }
            }
        }
    }
    // Lastly remove the original text leaving just our objects instead
    GetSubsystem<UI>()->GetRoot()->RemoveChild( currentText_ );
}

void DialogAboutUrho3D::ColouriseLetters( int x, int y)
{
    PhysicsWorld2D *physics  = scene_->GetComponent<PhysicsWorld2D>();
    Renderer       *renderer = GetSubsystem<Renderer>();

    // First get the maximum size of the flame

    // GetMinRadius() == 0, GetMaxRadius() == 100
    // getStartParticleSize() == 60, GetFinishParticleSize() == 5
    // None of these are any good, so just hardcoding another magic number
    //ParticleEffect2D* particleEffect = particleNode_->GetComponent<ParticleEmitter2D>()->GetEffect();
    float radius = MAGIC_NUMBER_FLAME_SIZE;

    // Now construct a rectangle centred on the mouse position of this radius;
    // unfortuantely we can't pass circles into the physics engine to get all
    // the rigidbodies inside, only rects, so it's a little innaccurate, but
    // we'll survive.

    Vector3 mouseLoc = renderer->GetViewport(0)->ScreenToWorldPoint( x, y, 10.0f);

    Rect   flameBounds = Rect( mouseLoc.x_ - radius, mouseLoc.y_ - radius,
                               mouseLoc.x_ + radius, mouseLoc.y_ + radius );

    PODVector<RigidBody2D*>  rigidBodies;

    physics->GetRigidBodies( rigidBodies, flameBounds );

    for (int k=0; k < (int)rigidBodies.Size(); k++)
    {
        Node *bodyNode = rigidBodies[k]->GetNode();

        StaticSprite2D* staticSprite = bodyNode->GetComponent<StaticSprite2D>();
        if (staticSprite)
        {
            Color    flame[] = {
                Color(0.6f,  0.0f,  0.0f ), // Dark Red
                Color(1.0f,  0.0f,  0.0f ), // Red
                Color(1.0f,  0.4f,  0.0f ), // Orange
                Color(1.0f,  1.0f,  0.0f ), // Yellow
                Color(1.0f,  1.0f,  0.8f ), // Light Yellow
                Color(0.95f, 0.95f, 0.95f), // Light Grey
            };

            Color    newcol = Color::WHITE;
            Color    col = staticSprite->GetColor();
            int      oldcol;
            Vector3  pos    = bodyNode->GetPosition();

            // Objects closest to the mouse cursor become dark red, then moving outwards
            // radially the order is red, orange, yellow, light yellow and light grey.
            // We only override if the new value is "hotter" than the previous one.

            float distance = sqrtf((pos.x_ - mouseLoc.x_) * (pos.x_ - mouseLoc.x_) +
                                   (pos.y_ - mouseLoc.y_) * (pos.y_ - mouseLoc.y_));

            for (oldcol=0; oldcol < sizeof(flame) / sizeof(Color); oldcol++)
            {
                if ((fabs(col.r_ - flame[oldcol].r_) < EPSILON) &&
                    (fabs(col.g_ - flame[oldcol].g_) < EPSILON) &&
                    (fabs(col.b_ - flame[oldcol].b_) < EPSILON))
                {
                    break;
                }
            }

            if (distance < radius / 6)
                newcol = (oldcol > 0) ? flame[0] : col;
            else if (distance < radius / 3)
                newcol = (oldcol > 1) ? flame[1] : col;
            else if (distance < radius / 2)
                newcol = (oldcol > 2) ? flame[2] : col;
            else if (distance < radius * 4 / 6)
                newcol = (oldcol > 3) ? flame[3] : col;
            else if (distance < radius * 5 / 6)
                newcol = (oldcol > 4) ? flame[4] : col;
            else
                newcol = (oldcol > 5) ? flame[5] : col;

            staticSprite->SetColor( newcol );
        }
    }
}

void DialogAboutUrho3D::CreateLogo()
{
    // Get logo texture
    ResourceCache* cache   = context_->GetSubsystem<ResourceCache>();
    Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/FishBoneLogo.png");
    if (!logoTexture)
        return;

    // Create logo sprite and add to the UI layout
    UI* ui = context_->GetSubsystem<UI>();

    SharedPtr<Sprite> logoSprite( ui->GetRoot()->CreateChild<Sprite>() );

    // Set logo sprite texture
    logoSprite->SetTexture(logoTexture);

    int textureWidth = logoTexture->GetWidth();
    int textureHeight = logoTexture->GetHeight();

    // Set logo sprite scale
    logoSprite->SetScale(256.0f / textureWidth * hiDpiScale_);

    // Set logo sprite size
    logoSprite->SetSize(textureWidth, textureHeight);

    // Set logo sprite hot spot - if it isn't clear what a hotspot is,
    // its the x,y co-ordinate in the image that the SetAlignment() operation
    // fixes to. I'd call this an anchor, but an anchor is something else for
    // Urho3D. As you can see the hotspot can actually be outside of the image.
    logoSprite->SetHotSpot(textureWidth/2, -60);

    // Set logo sprite alignment
    logoSprite->SetAlignment(HA_CENTER, VA_TOP);

    // Make logo not fully opaque to show the scene underneath
    logoSprite->SetOpacity(0.9f);

    // Set a high priority for the logo so that other UI elements are drawn beneath
    logoSprite->SetPriority(100);
}

// This is an exact copy of the 25_Urho2DParticle example
// in Urho3D so that it can be used as part of a larger application.
// We make this define so that the methods all come up in this class, and
// we comment out the include, constructor and Start() functions and a couple
// of other functions which are unused
#define Urho2DParticle DialogAboutUrho3D

//
// Copyright (c) 2008-2022 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
/*
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Urho2D/ParticleEffect2D.h>
#include <Urho3D/Urho2D/ParticleEmitter2D.h>

#include "Urho2DParticle.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(Urho2DParticle)

Urho2DParticle::Urho2DParticle(Context* context) :
    Sample(context)
{
}

void Urho2DParticle::Start()
{
    // Execute base class startup
    Sample::Start();

    // Set mouse visible
    auto* input = GetSubsystem<Input>();
    input->SetMouseVisible(true);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}
*/
void Urho2DParticle::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();

    // Create camera node
    cameraNode_ = scene_->CreateChild("Camera");
    // Set camera's position
    cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -10.0f));

    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetOrthographic(true);

    auto* graphics = GetSubsystem<Graphics>();
    camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);
    camera->SetZoom(1.2f * Min((float)graphics->GetWidth() / 1280.0f, (float)graphics->GetHeight() / 800.0f)); // Set zoom according to user's resolution to ensure full visibility (initial zoom (1.2) is set for full visibility at 1280x800 resolution)

    auto* cache = GetSubsystem<ResourceCache>();
    auto* particleEffect = cache->GetResource<ParticleEffect2D>("Urho2D/sun.pex");
    if (!particleEffect)
        return;

    particleNode_ = scene_->CreateChild("ParticleEmitter2D");
    auto* particleEmitter = particleNode_->CreateComponent<ParticleEmitter2D>();
    particleEmitter->SetEffect(particleEffect);

/*
    auto* greenSpiralEffect = cache->GetResource<ParticleEffect2D>("Urho2D/greenspiral.pex");
    if (!greenSpiralEffect)
        return;

    Node* greenSpiralNode = scene_->CreateChild("GreenSpiral");
    auto* greenSpiralEmitter = greenSpiralNode->CreateComponent<ParticleEmitter2D>();
    greenSpiralEmitter->SetEffect(greenSpiralEffect);
*/
}
/*
void Urho2DParticle::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    auto* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use mouse/touch to move the particle.");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}
*/
void Urho2DParticle::SetupViewport()
{
    auto* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}
/*
void Urho2DParticle::SubscribeToEvents()
{
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(Urho2DParticle, HandleMouseMove));
    if (touchEnabled_)
        SubscribeToEvent(E_TOUCHMOVE, URHO3D_HANDLER(Urho2DParticle, HandleMouseMove));

    // Unsubscribe the SceneUpdate event from base class to prevent camera pitch and yaw in 2D sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}
*/
void Urho2DParticle::HandleMouseMove(StringHash /* eventType */, VariantMap& eventData)
{
    if (particleNode_)
    {
        using namespace MouseMove;
        auto x = (float)eventData[P_X].GetInt();
        auto y = (float)eventData[P_Y].GetInt();
        auto* graphics = GetSubsystem<Graphics>();
        auto* camera = cameraNode_->GetComponent<Camera>();
        particleNode_->SetPosition(camera->ScreenToWorldPoint(Vector3(x / graphics->GetWidth(), y / graphics->GetHeight(), 10.0f)));
    }
}

// This is an exact copy of the 03_Sprites example
// in Urho3D so that it can be used as part of a larger application.
// We make this define so that the methods all come up in this class, and
// we comment out the include, constructor and Start() functions
#define Sprites DialogAboutUrho3D

//
// Copyright (c) 2008-2022 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

/*
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UI.h>

#include "Sprites.h"

#include <Urho3D/DebugNew.h>
*/
// Number of sprites to draw
static const unsigned NUM_SPRITES = 100;

// Custom variable identifier for storing sprite velocity within the UI element
static const StringHash VAR_VELOCITY("Velocity");

/*
URHO3D_DEFINE_APPLICATION_MAIN(Sprites)

Sprites::Sprites(Context* context) :
    Sample(context)
{
}

void Sprites::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the sprites to the user interface
    CreateSprites();

    // Hook up to the frame update events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}
*/

void Sprites::CreateSprites()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* graphics = GetSubsystem<Graphics>();
    auto* ui = GetSubsystem<UI>();

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // Get the Urho3D fish texture
    auto* decalTex = cache->GetResource<Texture2D>("Textures/UrhoDecal.dds");

    for (unsigned i = 0; i < NUM_SPRITES; ++i)
    {
        // Create a new sprite, set it to use the texture
        SharedPtr<Sprite> sprite(new Sprite(context_));
        sprite->SetTexture(decalTex);

        // The UI root element is as big as the rendering window, set random position within it
        sprite->SetPosition(Vector2(Random() * width, Random() * height));

        // Set sprite size & hotspot in its center
        sprite->SetSize(IntVector2(128, 128));
        sprite->SetHotSpot(IntVector2(64, 64));

        // Set random rotation in degrees and random scale
        sprite->SetRotation(Random() * 360.0f);
        sprite->SetScale(Random(1.0f) + 0.5f);

        // Set random color and additive blending mode
        sprite->SetColor(Color(Random(0.5f) + 0.5f, Random(0.5f) + 0.5f, Random(0.5f) + 0.5f));
        sprite->SetBlendMode(BLEND_ADD);

        // Add as a child of the root UI element
        ui->GetRoot()->AddChild(sprite);

        // Store sprite's velocity as a custom variable
        sprite->SetVar(VAR_VELOCITY, Vector2(Random(200.0f) - 100.0f, Random(200.0f) - 100.0f));

        // Store sprites to our own container for easy movement update iteration
        sprites_.Push(sprite);
    }
}

void Sprites::MoveSprites(float timeStep)
{
    auto* graphics = GetSubsystem<Graphics>();
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // Go through all sprites
    for (unsigned i = 0; i < sprites_.Size(); ++i)
    {
        Sprite* sprite = sprites_[i];

        // Rotate
        float newRot = sprite->GetRotation() + timeStep * 30.0f;
        sprite->SetRotation(newRot);

        // Move, wrap around rendering window edges
        Vector2 newPos = sprite->GetPosition() + sprite->GetVar(VAR_VELOCITY).GetVector2() * timeStep;
        if (newPos.x_ < 0.0f)
            newPos.x_ += width;
        if (newPos.x_ >= width)
            newPos.x_ -= width;
        if (newPos.y_ < 0.0f)
            newPos.y_ += height;
        if (newPos.y_ >= height)
            newPos.y_ -= height;
        sprite->SetPosition(newPos);
    }
}
/*
void Sprites::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Sprites, HandleUpdate));
}
*/
void Sprites::HandleUpdate(StringHash /* eventType */, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move sprites, scale movement with time step
    MoveSprites(timeStep);

    if (blowtorchFrame_ == 0)
    {
        // check for mouse release
        Input*  input = GetSubsystem<Input>();

        if (! input->GetMouseButtonDown(MOUSEB_LEFT))
            blowtorchFrame_ = 1;
    }
    // NOT else if

    if (blowtorchFrame_ > 0)
    {
        // -1 because the first frame is the blue frame for mouse down
        int num_frames = sizeof(btTexturePaths) / sizeof(const char *) - 1;
        // only change frame on every 10th update
        int animate_on = 10;

        // Our misuse of the cursorshape forces us to call this on EVERY update, otherwise
        // the engine restores CS_NORMAL.
        blowtorch_->SetShape( (CursorShape)(CS_NORMAL + (blowtorchFrame_ - 1) / animate_on + 1) );
        // index 0 is the blue flame we show when mouse is down, so we
        // perform the increment and mod the opposite way around to normal
        // to cycle between the 4 orange flames only
        blowtorchFrame_ = (blowtorchFrame_ % (num_frames * animate_on)) + 1;
    }
}
