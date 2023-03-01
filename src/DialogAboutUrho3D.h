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

#ifndef DLGABOUTURHO_H
#define DLGABOUTURHO_H

#include <QDialog>
#include <QTimer>

#include <Urho3D/Core/Context.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>

class DialogAboutUrho3D : public QDialog, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(DialogAboutUrho3D, Urho3D::Object)

public:
    DialogAboutUrho3D(QWidget *parent = nullptr);
    ~DialogAboutUrho3D();

// SDL uses a different implementation module on Windows than on Linux for
// its mouse functionality. Net result is that the mouse tracks properly in
// an Urho3d external window on Windows, but not on Linux. We have to map it
// through if we are on Linux, by capturing the mouse movements and clicks;
// we definitely do not want to do this on Windows where doing so doesn't just
// mess up mouse navigation but also causes a segfault on dialog close.
protected:
#if !defined(WIN32) && !defined(WIN64)
    // Have to override these so the mouse events can get to the Urho3D engine
    void        mouseMoveEvent(QMouseEvent *event)    override;
    void        mousePressEvent(QMouseEvent *event)   override;
    void        mouseReleaseEvent(QMouseEvent *event) override;
#endif

    void        closeEvent(QCloseEvent *event) override;

private slots:
    // Timeout handler.
    void OnTimeout();

private:
    // Use Urho3D naming convention for Urho3D member variables

    /// The handle to the graphics engine itself
    //  - it seems wasteful performance-wise to be starting and stopping
    //    engines like this every time we open or close a Window using
    //    Urho3D but it does seem to be the intent of the API to us it
    //    that way.
    Urho3D::SharedPtr<Urho3D::Engine>                     engine_;
    /// Vector to store the sprites for iterating through them.
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::Sprite> >    sprites_;
    /// SDL handle on the current window
    SDL_Window                                           *window_;

    /// whether mouse button is being held down or not
    int                                                   blowtorchFrame_;
    /// Blowtorch cursors
    Urho3D::SharedPtr<Urho3D::Cursor>                     blowtorch_;
    /// Particle scene node.
    Urho3D::SharedPtr<Urho3D::Node>                       particleNode_;
    /// Scene.
    Urho3D::SharedPtr<Urho3D::Scene>                      scene_;
    /// Camera scene node.
    Urho3D::SharedPtr<Urho3D::Node>                       cameraNode_;
    /// The DPI scale: 1.0 if lowdpi, probably 2.0 for Retina displays
    double                                                hiDpiScale_;
    
    // And use the QT nameing converntion for the QT member variables
    QTimer                    m_timer;
    int                       m_numPages;
    int                       m_currentPage;
    const char               *m_pages[10];
    int                       m_totalLetters;
    int                       m_fallenLetters;
    int                       m_drawTextTimer;

    // Breakup the authors text into pages
    void CreateTextPages();

    // The Urho3D logo
    void CreateLogo();
    /// Construct the scene content.
    void CreateScene();
    /// plane for letters to land on when they fall
    void CreateGroundLevel();
    /// Set 'about' text to the License
    void CreateText( const char *text );
    /// Set up a viewport for displaying the scene.
    void SetupViewport();
    /// Mouse click callback from Urho3D
    void HandleMouseClick(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Wrap the mouse move notification in the particle demo so it only emits when mouse down
    void HandleMouseMoveWrap(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Wrap the update notification in the sprite demo so we can get updates too
    void HandleUpdateWrap(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    /// Construct the sprites.
    void CreateSprites();
    /// Move the sprites using the delta time step given.
    void MoveSprites(float timeStep);
    /// Subscribe to application-wide logic update events.
    void SubscribeToEvents();
    /// Handle the logic update event.
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    /// Change the colour of letters under the flame centred at x, y screen co-ordinates
    void ColouriseLetters( int x, int y );
    /// The Letters colliding notifier
    void HandleLetterCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// The Page Down button handler
    void HandlePageDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// The Ok button handler
    void HandleClose(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// The particle following the mouse
    void HandleMouseMove(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::Text   *currentText_;
};

#endif
