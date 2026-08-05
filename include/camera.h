#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };
enum Camera_Type { FREE, FPS, STATIONARY};

// Default camera values
// ---------------------
// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction
// vector pointing to the right so we initially rotate a bit to the left.
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;  // fov

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
   public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 OriginalPosition;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    Camera_Type CameraType;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
           float pitch = PITCH, Camera_Type cameraType = FREE)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM),
          CameraType(cameraType) {
        OriginalPosition = position;
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
           float yaw, float pitch, Camera_Type cameraType)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM),
          CameraType(cameraType) {
        OriginalPosition = glm::vec3(posX, posY, posZ);
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt
    // Matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts
    // input parameter in the form of camera defined ENUM (to abstract it from
    // windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        if (CameraType == FPS) {
            // make sure the user stays at the ground level
            Position.y = 1.75f;  // <-- this one-liner keeps the user at the
            // ground level (xz plane)
        } else if (CameraType == STATIONARY) {
            return;
        }
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT) Position -= Right * velocity;
        if (direction == RIGHT) Position += Right * velocity;
    }

    void UpdatePosition( glm::mat4 mat) {
        Position = glm::vec3(mat * glm::vec4(OriginalPosition, 1.0f));
    }

    void setPos(float pos[]) {
        // OriginalPosition.x = pos[0];
        // OriginalPosition.y = pos[1];
        // OriginalPosition.z = pos[2];
        Position.x = pos[0];
        Position.y = pos[1];
        Position.z = pos[2];
    }

    // processes input received from a mouse input system. Expects the offset
    // value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset,
                              GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        // make sure that when pitch is out of bounds, screen doesn't get
        // flipped
        if (constrainPitch) {
            Yaw = glm::mod(Yaw + xoffset, 360.0f);
            Pitch = glm::clamp(Pitch + yoffset, -89.0f, 89.0f);
        } else {
            Yaw += xoffset;
            Pitch += yoffset;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires
    // input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    }

   private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = static_cast<float>(cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
        front.y = static_cast<float>(sin(glm::radians(Pitch)));
        front.z = static_cast<float>(sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(
            Front, WorldUp));  // normalize the vectors, because their length
                               // gets closer to 0 the more you look up or down
                               // which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
