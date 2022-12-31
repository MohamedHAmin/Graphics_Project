#pragma once

#include "../ecs/world.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace our
{
    struct lock{
        bool 
        forward = false,
        left = false,
        right = false,
        backward = false;
    };
    class PlayerMovementSystem {
    public:
        Application* app; // The application in which the state runs
        Entity* player = nullptr;
        std::unordered_map<Entity*, bool> collisions;
        std::unordered_map<Entity*, lock> entityLocking;
        float facing = 180;
        const float deathAngular = 5;
        const float speed = 10;
        const glm::vec4 velocity = glm::vec4(0, 0, 1, 0);
        bool playing = true;
        bool 
        forward = true,
        left = true,
        right = true,
        backward = true;
   
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        void update(World* world, float deltaTime) {
            if (!player){
                for(auto entity : world->getEntities()){
                    if (!player && entity->name == "player") player = entity;
                }
            }

            if(!player) return;

            if (!playing){
                if (left){
                    if (player->localTransform.rotation.z > -glm::pi<float>() / 2.0f) player->localTransform.rotation.z -= deathAngular * deltaTime;
                }else if (right){
                    if (player->localTransform.rotation.z < glm::pi<float>() / 2.0f) player->localTransform.rotation.z += deathAngular * deltaTime;
                }else if (forward){
                    if (player->localTransform.rotation.x > -glm::pi<float>() / 2.0f) player->localTransform.rotation.x -= deathAngular * deltaTime;
                }else{
                    if (player->localTransform.rotation.x < glm::pi<float>() / 2.0f) player->localTransform.rotation.x += deathAngular * deltaTime;
                }
                return;
            }

            for(auto entity : world->getEntities()){
                if (entity->name == "floor" || entity->name == "light" || entity == player) continue;
                glm::vec3 entityCenter = entity->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
                glm::vec3 playerCenter = player->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
                float distance = glm::distance(entityCenter, playerCenter);
                if (distance < 1.3){
                    onCollisionEnter(entity, entityCenter, playerCenter);       
                }else if (collisions.find(entity) != collisions.end() && collisions[entity]){
                    onCollisionExit(entity, entityCenter, playerCenter);
                }
            }

            if(app->getKeyboard().isPressed(GLFW_KEY_W) && forward) facing = glm::pi<float>();
            else if(app->getKeyboard().isPressed(GLFW_KEY_S) && backward) facing = 0.0f;
            else if(app->getKeyboard().isPressed(GLFW_KEY_D) && right) facing = glm::pi<float>() / 2.0f;
            else if(app->getKeyboard().isPressed(GLFW_KEY_A) && left) facing = -glm::pi<float>() / 2.0f;
            else return;
            player->localTransform.rotation.y = facing;
            player->localTransform.position += glm::vec3(speed) * glm::vec3(deltaTime * glm::yawPitchRoll(facing, 0.0f, 0.0f) * velocity);
        }

        void onCollisionEnter(Entity* entity, glm::vec3 entityCenter, glm::vec3 playerCenter){
            glm::vec3 dir = entityCenter - playerCenter;
            if (dir.x > 0){ right = false; entityLocking[entity].right = true;}
            else if (dir.x < 0){ left = false; entityLocking[entity].left = true;}
            if (dir.z > 0){ backward = false; entityLocking[entity].backward = true;}
            else if (dir.z < 0){ forward = false; entityLocking[entity].forward = true;}
            if (entity->name == "car"){
                playing = false;
            }
            collisions[entity] = true;
        }

        void onCollisionExit(Entity* entity, glm::vec3 entityCenter, glm::vec3 playerCenter){
            right ^= entityLocking[entity].right;
            left ^= entityLocking[entity].left;
            backward ^= entityLocking[entity].backward;
            forward ^= entityLocking[entity].forward;
            entityLocking[entity] = {0, 0, 0, 0};
            collisions[entity] = false;
        } 

        void exit(){
            
        }
    };

}