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

// Copied from the Character.cpp file in the CharacterDemo Urho3D example, with
// some minor additions to allow for a 'noclip' mode, ability to do both walking
// and running and the fighting animations (because I thought they looked cool
// even if not serving a practical purpose.)
// The class was renamed to avoid confusion with the existing character class.

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/ThirdParty/Bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "Avatar.h"

Avatar::Avatar(Context* context) :
    LogicComponent(context),
    onGround_(false),
    okToJump_(true),
    inAirTimer_(0.0f)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Avatar::RegisterObject(Context* context)
{
    context->RegisterFactory<Avatar>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("On Ground", bool, onGround_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("OK To Jump", bool, okToJump_, true, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Air Timer", float, inAirTimer_, 0.0f, AM_DEFAULT);
}

void Avatar::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Avatar, HandleNodeCollision));
}

void Avatar::FixedUpdate(float timeStep)
{
    /// \todo Could cache the components for faster access instead of finding them each frame
    auto* body = GetComponent<RigidBody>();
    auto* animCtrl = node_->GetComponent<AnimationController>(true);

    float speed       = 1.0f;
    bool  fighting    = false;
    bool  noclip_mode = false;

    if (body->GetCollisionEventMode() == COLLISION_NEVER)
        noclip_mode = true;

    // Update the in air timer. Reset if grounded
    if (!onGround_ && !noclip_mode)
        inAirTimer_ += timeStep;
    else
        inAirTimer_ = 0.0f;
    // When character has been in air less than 1/10 second, it's still interpreted as being on ground
    bool softGrounded = inAirTimer_ < INAIR_THRESHOLD_TIME;

    // Update movement & animation
    //const Quaternion& rot = node_->GetRotation();

    // Calculate the floor angle with the floor normal and rotate the character with that angle so it can move on any direction and have vertical velocity.
    // Based on 1vanK's post: https://discourse.urho3d.io/t/solved-how-to-direct-a-character-parallel-to-the-ground/1285/5
    Quaternion floorAngle = Quaternion(Vector3(0.0f, 1.0f, 0.0f), floorNormal_);
    Quaternion rot = floorAngle * Quaternion(node_->GetRotation());

    Vector3 moveDir = Vector3::ZERO;
    const Vector3& velocity = body->GetLinearVelocity();
    // Velocity on the XZ plane
    Vector3 planeVelocity(velocity.x_, (noclip_mode ? velocity.y_ : 0.0f), velocity.z_);

    // The planeVelocity is a BRAKING force - in order to go faster we multiply the ApplyImpulse()
    // call below instead.

    if (controls_.IsDown(CTRL_RUN))
        speed *= 3;
    if (controls_.IsDown(CTRL_FORWARD))
        moveDir += Vector3::FORWARD;
    if (controls_.IsDown(CTRL_BACK))
        moveDir += Vector3::BACK;
    if (controls_.IsDown(CTRL_STRAFE_LEFT))
        moveDir += Vector3::LEFT;
    if (controls_.IsDown(CTRL_STRAFE_RIGHT))
        moveDir += Vector3::RIGHT;
    if (controls_.IsDown(CTRL_PUNCH))
        animCtrl->PlayExclusive("Models/Mutant/Mutant_Punch.ani", 0, false, 0.2f);
    if (controls_.IsDown(CTRL_SWIPE))
        animCtrl->PlayExclusive("Models/Mutant/Mutant_Swipe.ani", 0, false, 0.2f);
    if (controls_.IsDown(CTRL_KICK))
        animCtrl->PlayExclusive("Models/Mutant/Mutant_Kick.ani", 0, false, 0.2f);

    if (animCtrl->IsPlaying("Models/Mutant/Mutant_Punch.ani") ||
        animCtrl->IsPlaying("Models/Mutant/Mutant_Swipe.ani") ||
        animCtrl->IsPlaying("Models/Mutant/Mutant_Kick.ani"))
    {
        // SetRemoveOnCompletion() doesn't work! And we can't replay an animation if
        // it still thinks it is playing but sitting at the end
        if (animCtrl->IsAtEnd("Models/Mutant/Mutant_Punch.ani") ||
            animCtrl->IsAtEnd("Models/Mutant/Mutant_Swipe.ani") ||
            animCtrl->IsAtEnd("Models/Mutant/Mutant_Kick.ani"))
        {
            // Calling Stop() puts the model back into a T position, so better to call
            // idle, and let it be overridden by something below if necessary
            // animCtrl->Stop("Models/Mutant/Mutant_Swipe.ani");
            animCtrl->PlayExclusive("Models/Mutant/Mutant_Idle0.ani", 0, true, 0.2f);
            fighting = false;
        }
        else
        {
            fighting = true;
        }
    }

    // Normalize move vector so that diagonal strafing is not faster
    if (moveDir.LengthSquared() > 0.0f)
        moveDir.Normalize();

    if (noclip_mode)
    {
        // In no clip mode we can move in ALL directions, so apply the full vector of the camera.
        // Also we lose all kind of inertia variables and whatnot, so multiply it a bit to subjectively
        // take it back to the speed we get out of noclip mode. Technically I think it is a bit faster
        // but it really doesn't appear to be when you're using it - seems slower.
        body->SetLinearVelocity( rot * Quaternion(controls_.pitch_, Vector3::RIGHT) * moveDir * MOVE_FORCE * 8 * speed);
        body->GetBody()->activate(true);
        if (!fighting)
            animCtrl->PlayExclusive("Models/Mutant/Mutant_Idle1.ani", 0, false, 0.2f);
    }
    else
    {
        // If in air, allow control, but slower than when on ground
        body->ApplyImpulse(rot * moveDir * (softGrounded ? MOVE_FORCE : INAIR_MOVE_FORCE) * speed);
        if (softGrounded)
        {
            // When on ground, apply a braking force to limit maximum ground velocity
            Vector3 brakeForce = -planeVelocity * BRAKE_FORCE;
            body->ApplyImpulse(brakeForce);

            // Jump. Must release jump control between jumps
            if (controls_.IsDown(CTRL_JUMP))
            {
                if (okToJump_)
                {
                    body->ApplyImpulse(Vector3::UP * JUMP_FORCE);
                    okToJump_ = false;
                    animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
                }
            }
            else
                okToJump_ = true;
        }

        if ( !onGround_ )
        {
            animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
        }
        else
        {
            // Play walk animation if moving on ground, otherwise fade it out
            if (softGrounded && !moveDir.Equals(Vector3::ZERO))
            {
                // Set walk animation speed proportional to velocity
                if (speed == 1.0)
                {
                    animCtrl->PlayExclusive("Models/Mutant/Mutant_Walk.ani", 0, true, 0.2f);
                    animCtrl->SetSpeed("Models/Mutant/Mutant_Walk.ani", planeVelocity.Length() * 0.5f);
                }
                else
                {
                    animCtrl->PlayExclusive("Models/Mutant/Mutant_Run.ani", 0, true, 0.2f);
                    animCtrl->SetSpeed("Models/Mutant/Mutant_Run.ani", planeVelocity.Length() * 0.3f);
                }
            }
            else if (!fighting)
            {
                animCtrl->PlayExclusive("Models/Mutant/Mutant_Idle0.ani", 0, true, 0.2f);
            }
        }
        if (!noclip_mode)
        {
            // Stops the character from sliding down when not moving.
            // From 1vanK's post: https://discourse.urho3d.io/t/improved-charactercontroller/3472
            if ( softGrounded && okToJump_ && moveDir == Vector3::ZERO)
            {
                body->SetUseGravity(false);
                body->SetLinearVelocity(Vector3::ZERO);
            }
            else
            {
                body->SetUseGravity(true);
            }
        }
    }

    // Reset grounded flag for next frame
    onGround_ = false;
}

void Avatar::HandleNodeCollision(StringHash /* eventType */, VariantMap& eventData)
{
    // Check collision contacts and see if character is standing on ground (look for a contact that has near vertical normal)
    using namespace NodeCollision;

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        /*float contactDistance = */contacts.ReadFloat();
        /*float contactImpulse = */contacts.ReadFloat();

        // If contact is below node center and pointing up, assume it's a ground contact
        if (contactPosition.y_ < (node_->GetPosition().y_ + 1.0f))
        {
            float level = contactNormal.y_;
            if (level > 0.75)
                onGround_ = true;
        }
    }
}
