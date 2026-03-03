/* Start Header ************************************************************************/
/*!
\file        Component.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 40%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
\author      Asha Mathyalakan, asha.m, 2402886, 15%
\author      Tan Chee Qing, cheeqing.tan, 2401486, 10%
             Total - 100%
\par         s.sinle@digipen.edu
\par         low.h@digipen.edu
\par		 p.lin@digipen.edu
\par         asha.m@digipen.edu
\par         cheeqing.tan@digipen.edu
\date        October, 1st, 2025
\brief       This file contains all the component definitions for the Component-based architecture.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <memory> //for unique_ptr
#include <unordered_set>
#include "renderer.h"
#include "mathlib.h"
#include "font.h"
#include "dynamics.h"
#include "queue"


// at least it works!!!
namespace JsonIO {
    // for the indexing in animation vector
    inline std::vector<std::string> stateNames = { "Idle", "Walking", "Jumping", "Falling", "Shooting", "Dead" };
}

// basic component class
class Component {
public:
    // virtual just means that the child class can overwrite 
    // these functions
    virtual ~Component() = default;

    virtual std::unique_ptr<Component> clone() const = 0;
};

// Transform component to store position, rotation, and scale
struct Transform : public Component {
public:
    Transform() = default;
    Transform(float px, float py, float pz, float rot, float sx, float sy, float sz)
        : x(px), y(py), z(pz), rotation(rot), scaleX(sx), scaleY(sy), scaleZ(sz), mdlWorld(1) {
    }

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Transform>(*this);
    }

    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ{ 0.f };
    bool flipX = false; // flip scaleX when change direction, true means flip to left
    glm::mat4 mdlWorld = 1.f;

    // Hierarchy (optional, null = root object)
    std::string parentName = "";   // serialised as name reference
    Vector2D    localOffset{ 0, 0 }; // position relative to parent at bind time
    float       localRotation = 0.0f; // rotation relative to parent at bind time
};

// Render component to store rendering-related properties
struct Render : public Component {
public:
    Render() : texHDL{ 0 } {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Render>(*this);
    }

public:
    bool visible = true;
    bool hasTex = false;
    bool isTransparent = false;
    bool hasAnimation = false;
    //bool wireframe = false;
    // sorry but i need this for real time texture update
    // wait for boss sin le to help me settle this - kelly :)
    bool texChanged = false;
    GLuint texHDL;
    std::string texFile;
    glm::vec4 clr{ 1,1,1,1 };
    renderer::model modelRef;
};

struct camera : public Component
{
    public:
    camera() = default;
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<camera>(*this);
    }
    bool useScreenSize = true;
    float height = 1.f;
    int width = 1;
    float zoom = 10;
    float minZoom = 0;
	float maxZoom = 50;
    float mapWidth = 1;
    float mapHeight = 1;
};

struct Particle : public Component {
public:
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Particle>(*this);
    }

public:
    struct GroupData 
    {
        glm::vec4 clr{ 1,1,1,1 };
        Vector2D velocity{ 0,0 };
        float timer = 0;
        float offset = 0;
        int particleAmt = 0;
        bool isActive = false;
    };
    struct individualData
    {
        Vector2D velocity{ 0,0 };
        float timer = 0;
        size_t grpNo = 0;
    };
    struct UID
    {
        size_t uid = 0;
        size_t particleAmt = 0;
    };
    //size_t grpNo = 0;
    //bool activate = false;
    bool isMaster = true;
    individualData childData;
    GroupData master;
    static std::vector<UID>UIDs;
};

struct FontComponent : public Component {
    FontComponent() = default;

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<FontComponent>(*this);
    }

    //GLuint texHdl;
    //Font::FontMdl mdl = Font::fontMdls[0];
    Font::FontMdl mdl{};
    std::string word = "";
    float scale = 1;
    glm::vec3 clr{ 0.f,0.f,0.f };
    int fontType = 1;

    std::string fontPath = "assets/ARIAL.TTF";
};

// animation for different state
struct AnimateState {
    bool loop = true;
	bool play = false;
    Vector2D initialFrame{}, lastFrame{};
    int currentFrameColumn{}, currentFrameRow{}, totalColumn{ 1 }, totalRow{ 1 };
    float frameTime = 0.1f, frameTimer{};
    GLuint texHDL{};
    std::string texFile{};

    // do not serialize this
    bool texChanged = false;
};

struct Animation : public Component
{
    Animation() { animState.resize(JsonIO::stateNames.size()); } // set animState to be of equal size of statenames
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Animation>(*this);
    }

    bool runItBack = true;
    std::vector<AnimateState> animState; // to store the animation data for dff state, index by playerstate
};

// Physics component to store velocity and other physics-related properties
struct Physics : public Component {
public:
    //clone component
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Physics>(*this);
    }

	//helper function to recalculate moment of inertia when collider size changes, for now just use box collider formula, can add more later if needed
    void recalculateInertia(const Vector2D& colliderSize, const shape colliderType);

    bool physicsFlag = true;
    float moveSpeed = 1.0f;

    Dynamics dynamics;

    bool canMove = true;  // Add this flag
    bool autoUpdate = false;

    float velX = 0.0f;
    float velY = 0.0f;
    float gravity = -20.f;

    // properties
    float damping = 0.98f; // in the air
    //float friction = 0.2f; // on ground

    //Jump 
    float jumpForce = 10.0f;
    float floorY = -7.5f;
    bool onGround = false;
    float maxFallSpeed = -20.f;
    float groundAccelForce = 20.f;
    float airAccelForce = 8.f;
    float apexHangMul = 0.2f;

	// jump buffer (forgive early jump before landing)
	float jumpBufferTime = 0.15f; // how long before landing can still jump
	float jumpBufferTimer = 0.f;

    // coyote jump (can still jump after walking off ledge)
    float coyoteTime = 0.35f;      // how long after leaving edge can still jump
    float coyoteTimer = 0.f;
    bool  coyoteAvailable = false;

    // ledge assist
	float ledgeAssistRange = 0.8f; // how far away from ledge can still grab
    float ledgeClimbSpeed = 1.f;
    float ledgeTargetY = 0.f;
    bool ledgeClimb = false;

    //water bullet
    bool alive = false;
    float lifeTimer = 0.0f;
    float maxLifetime = 3.0f;
    Vector2D originalPos{ 0, 0 };
    Vector2D originalVel{ 0, 0 };
    bool isOriginalStateSet = false;

    //bool inWaterTriggered; // true only when W is pressed

    //Shooting parameters
    float fireRate = 0.15f;        // Time between shots (lower is  faster)
    float shootTimer = 0.0f;       // Internal time for shooting
    bool canShoot = true;
    bool autoFire = true;         
    // Arc parameters
    float arcHeightFactor = 0.4f;
    bool useArcTrajectory = true;
    float minArcHeight = 2.0f;

    bool inWater = false;
    bool buoyancy = true;

    //bullet gravity thing
    float gravityDelayTimer = 0.0f;
    float gravityDelay = 0.2f;

	bool freezeRotation = true; //determine whether object can rotate (by default cannot)
	float inertiaOverride = 0.0f; // if > 0, use this value for inertia instead of calculating based on mass and collider size
    

    //const float waterBounce = 60.0f;   // upward velocity when entering water

};

// Input component to store input-related properties
struct Input : public Component {
public:
    //Input(float speed = 0.0f) : moveSpeed(speed) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Input>(*this);
    }

public:
    //float moveSpeed = 0.0f;

};

// enum class used for CollisionSystem, to determine to stop/push by force (currently not force ig)
enum class CollisionResponseMode {
    StopWhenCollide, // if collide by another obj, just stop
    MoveWhenCollide, // if collide by another obj, apply "fake" force and kena push
    NoResponse
};

//button action 
enum class ButtonAction {
    NONE,
    START_GAME,
    OPEN_CONTROL,
	OPEN_AUDIO,
    EXIT_GAME,
    RESUME_GAME,
	YES_EXIT,
	NO_EXIT,
    BACK,
    MENU,
    MUSIC_VOL_UP,
    MUSIC_VOL_DOWN,
    SFX_VOL_UP,
    SFX_VOL_DOWN
};

//Button component
struct Button : public Component {
    ButtonAction actionName = ButtonAction::NONE;   //action this button triggers
    bool isHovered = false;   //hover state
    bool isPressed = false;   //pressed state

    Button() = default;
    Button(ButtonAction action) : actionName(action) {} 

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Button>(*this);
    }
};

// to determine how to combine bounciness and friction when two objects collide, for now just use average, can add more later if needed
struct PhysicsMaterial {
    enum class CombineMode { Average, Minimum, Maximum, Multiply };

    float bounciness = 0.0f;
    float friction = 0.f; // higher means harder to move, rmb to 1 - friction for movement
    CombineMode bounceCombine = CombineMode::Average;
    CombineMode frictionCombine = CombineMode::Average;

    static float combine(float a, float b, CombineMode mode) {
        switch (mode) {
        case CombineMode::Minimum:  return std::min(a, b);
        case CombineMode::Maximum:  return std::max(a, b);
        case CombineMode::Multiply: return a * b;
        default:                    return (a + b) * 0.5f; // Average
        }
    }

    float combinedBounciness(const PhysicsMaterial& other) const {
        CombineMode mode = (bounceCombine > other.bounceCombine)
            ? bounceCombine : other.bounceCombine; // higher priority wins
        return combine(bounciness, other.bounciness, mode);
    }

    float combinedFriction(const PhysicsMaterial& other) const {
        CombineMode mode = (frictionCombine > other.frictionCombine)
            ? frictionCombine : other.frictionCombine;
        return combine(friction, other.friction, mode);
    }
};

// CollisionInfo component to store collision-related information
struct CollisionInfo : public Component {
public:

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<CollisionInfo>(*this);
    }


    bool collisionFlag = true; // whether objest can collide with another obj
    shape colliderType = shape::square;
    Vector2D colliderSize{ 0,0 }; // size of the bounding box, if collider is circle then x is diamater
    Vector2D colliderPos{ 0,0 };
    float colliderRot = 0.f;
    bool autoFitScale = true; // auto-fit the collider with the size of the mesh (i.e. obj bigger -> collider bigger)
    CollisionResponseMode collisionRes = CollisionResponseMode::StopWhenCollide;

    std::string collisionTag = "default";  // Tag for object
    std::unordered_set<std::string> ignoreCollisionsWith;  // Tags to ignore collision with
	PhysicsMaterial material; // Physical properties for collision response

    // Helper method to check if should collide with another tag
    bool shouldCollideWith(const std::string& otherTag) const {
        return ignoreCollisionsWith.find(otherTag) == ignoreCollisionsWith.end();
    }

    // run-time calculation, do not de/serialize
    bool collided = false; // check whether obj alr collided
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
    Vector2D contactPoint = { 0,0 };
    float penetration = 0.0f; //for it to not overlap 

    /*float bounciness = 0.0f;
    float friction = 0.0f;*/

};

// For imgui(testing)
struct UIComponent : public Component {
    //idk empty as of now
    //idk how i can use it
};

//for lua scripting
//to know which object to run
struct LuaScript : public Component {
public:
    LuaScript() {
        scripts.push_back(ScriptEntry());
    }

    LuaScript(const std::string& file) {
        scripts.push_back(ScriptEntry(file));
    }
    //LuaScript(const std::string& file)
    //    : filename(file) {
    //    // Extract just the filename without extension to use as scriptName
    //    size_t lastSlash = file.find_last_of("/\\");
    //    std::string fname = (lastSlash != std::string::npos) ? file.substr(lastSlash + 1) : file;

    //    size_t lastDot = fname.find_last_of('.');
    //    scriptName = (lastDot != std::string::npos) ? fname.substr(0, lastDot) : fname;
    //}

    std::unique_ptr<Component> clone() const override {
        auto cloned = std::make_unique<LuaScript>(*this);
        cloned->scriptsChanged = true; // Always reload for cloned components
        return cloned;
    }

    struct ScriptEntry {
        std::string filename = "";   // Full path to the Lua file
        std::string scriptName = ""; // Just the object name (e.g., "bullet", "player")
        bool changed = true;

        ScriptEntry() = default;
		ScriptEntry(const std::string& file)
			: filename(file) {
			// Extract just the filename without extension to use as scriptName
			size_t lastSlash = file.find_last_of("/\\");
			std::string fname = (lastSlash != std::string::npos) ? file.substr(lastSlash + 1) : file;
			size_t lastDot = fname.find_last_of('.');
			scriptName = (lastDot != std::string::npos) ? fname.substr(0, lastDot) : fname;
		}
    };

    //std::string filename;   // Full path to the Lua file
    //std::string scriptName; // Just the object name (e.g., "bullet", "player")
	std::vector<ScriptEntry> scripts;
	bool scriptsChanged = true; // To track if the script file has changed
};

// Logic Container (FSM)
enum class PlayerState { Idle, Walking, Jumping, Falling, Shooting, Dead };

struct StateMachine : public Component {
    PlayerState state = PlayerState::Idle;
    float       stateTime = 0.f;
    bool        facingRight = true;
    bool        fixState = false;
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<StateMachine>(*this);
    }
};

//Audio component
struct FadeInfo {
    bool isFading = false;
    bool isFadingIn = false;
    float startVolume = 0.0f;
    float targetVolume = 0.0f;
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;

    float defaultFadeInDuration = 1.0f;
    float defaultFadeOutDuration = 1.0f;
};

enum class AudioState {
    Stopped,
    Playing,
    Paused
};

//AudioChannel serves as the real storage for sounds and their data.
struct AudioChannel {
    FMOD::Channel* channel = nullptr;
    std::string audioFile;
    AudioState state = AudioState::Stopped;

    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool muted = false;

    bool fadeInOnStart = false;
    float fadeInDuration = 1.0f;
    bool fadeOutOnStop = false;
    float fadeOutDuration = 1.0f;
    FadeInfo fadeInfo;

    bool isPendingPlay = false;
    bool isPendingStop = false;
    bool playOnStart = false;

    //new stuff for spatial audio
	bool isSpatial = false;
	float minDistance = 1.0f;
	float maxDistance = 100.0f;
};

//enum class soundID : int;
//enum class AudioState;
class FMOD::Channel;

//Audio component stores multiple Audio channels
//sort of acts as a manager of AudioChannels
struct AudioComponent : public Component {
public:
    AudioComponent() = default;

    /*AudioComponent(const std::string& file, float vol = 1.0f, bool shouldLoop = false, bool playStart = false)
        : audioFile(file)
        , volume(vol)
        , loop(shouldLoop)
        , playOnStart(playStart)
    {}*/
    //need to change this to make it work with multiple sounds

    AudioComponent(const std::string& file, float vol = 1.0f,
        bool shouldLoop = false, bool playStart = false)
    {
        // Create "default" channel with these parameters
        AudioChannel& ch = audioChannels["default"];
        ch.audioFile = file;
        ch.volume = vol;
        ch.loop = shouldLoop;
        ch.playOnStart = playStart;
    }

    std::unique_ptr<Component> clone() const override {
        auto cloned = std::make_unique<AudioComponent>();

        for (const auto& pair : audioChannels) {
            cloned->audioChannels[pair.first] = pair.second;
            cloned->audioChannels[pair.first].channel = nullptr;
            cloned->audioChannels[pair.first].state = AudioState::Stopped;
            cloned->audioChannels[pair.first].fadeInfo.isFading = false;
        }

        return cloned;
    }

    // Audio ID - which sound to play
    //std::string audioFile;

    //prolly need a map for 1 component to multiple sounds
    std::map<std::string, AudioChannel> audioChannels;

    //soundID soundId = static_cast<soundID>(0);

    AudioChannel* getChannel(const std::string& channelName) {
        auto it = audioChannels.find(channelName);
        if (it != audioChannels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    const AudioChannel* getChannel(const std::string& channelName) const {
        auto it = audioChannels.find(channelName);
        if (it != audioChannels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    //get or create channel
    AudioChannel* getOrCreateChannel(const std::string& name) {
        return &audioChannels[name];
    }

    //check if channel exists
    bool hasChannel(const std::string& name) const {
        return audioChannels.find(name) != audioChannels.end();
    }

    //remove a channel
    void removeChannel(const std::string& name) {
        auto it = audioChannels.find(name);
        if (it != audioChannels.end()) {
            //stop the channel if playing
            if (it->second.channel) {
                it->second.isPendingStop = true;
            }
            audioChannels.erase(it);
        }
    }

    //idk if we need this
    AudioChannel* getDefaultChannel() {
        return getOrCreateChannel("default");
    }

    void updateFades(float deltaTime) {
		for (auto& pair : audioChannels) {//updated for channel map
            AudioChannel& ch = pair.second;

            if (!ch.fadeInfo.isFading || !ch.channel) continue;//bugfix

            ch.fadeInfo.fadeTimer += deltaTime;

            float progress = ch.fadeInfo.fadeTimer / ch.fadeInfo.fadeDuration;
            if (progress > 1.0f) progress = 1.0f;

            float currentVolume;
            if (ch.fadeInfo.isFadingIn) {
                currentVolume = ch.fadeInfo.startVolume + (ch.fadeInfo.targetVolume - ch.fadeInfo.startVolume) * progress;
                //std::cerr << "sound fading" << currentVolume << std::endl;
            }
            else {
                currentVolume = ch.fadeInfo.startVolume + (ch.fadeInfo.targetVolume - ch.fadeInfo.startVolume) * progress;
            }

            FMODErrorCheck(ch.channel->setVolume(currentVolume));

            if (progress >= 1.0f) {
                ch.fadeInfo.isFading = false;

                if (!ch.fadeInfo.isFadingIn && ch.fadeInfo.targetVolume == 0.0f) {
                    ch.channel->stop();
                    ch.channel = nullptr;
                }
            }

        }

    }
};

// tile map component
struct TileMap : public Component
{
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<TileMap>(*this);
    }

    //std::string filename{};
    size_t tilesetTexID = 0;

    //how big is the tile
    float tileW = 3.0f;
    float tileH = 3.0f;

    int rows = 50;
    int columns = 50;

    bool enableCol = false;
	bool colChanged = true; // flag to rebuild collision box

    //size_t tilesetRows = 1, tilesetCols = 1;

    struct TileKey {
        int x;
        int y;

        bool operator==(const TileKey& other) const {
            return x == other.x && y == other.y;
        }
    };

    struct TileKeyHash {
        std::size_t operator()(const TileKey& k) const noexcept {
            return (std::hash<int>()(k.x) << 1) ^ std::hash<int>()(k.y);
        }
    };
    std::unordered_map<TileKey, std::string, TileKeyHash> tiles;

    struct TileCollisionBox {
        int x = 0, y = 0, width = 0, height = 0; // tile coordinates and size
        Vector2D minAABB = Vector2D(0.f, 0.f);
        Vector2D maxAABB = Vector2D(0.f, 0.f);
    };
    std::vector<TileCollisionBox> collisionBoxs; // merged collision box

    std::string getTile(int x, int y) const {
        TileKey key{ x, y };
        auto it = tiles.find(key);
        return (it != tiles.end()) ? it->second : ""; // -1 = empty
    }

    void setTile(int x, int y, std::string tileID) {
        if (x >= -columns && x < columns && y >= -rows && y < rows) {
            tiles[{x, y}] = tileID;
            colChanged = true;
        }
    }

    void clearTile(int x, int y) {
        tiles.erase({ x, y });
        colChanged = true;
    }
};

// virtual logic container (editor)
struct LogicConnection {
	int fromNodeId = 0;
	int toNodeId = 0;

    std::string outputPin; // OUT, TRUE, FALSE
    std::string inputPin; // IN

    inline bool operator==(const LogicConnection& other) const {
        return fromNodeId == other.fromNodeId && toNodeId == other.toNodeId &&
            outputPin == other.outputPin && inputPin == other.inputPin;
    }
};

enum class NodeType {
    EVENT,
    ACTION,
    CONDITION,
    DEFAULT // nothing happen
};

enum class NodeImplementation {
    // EVENT
    ON_START,
    ON_UPDATE,
    ON_COLLISION_ENTER,
    WHILE_COLLIDING,
    ON_COLLISION_EXIT,
    ON_HOVER_ENTER,
    WHILE_HOVERING,
	ON_HOVER_EXIT,
    ON_CLICK_ENTER,
    WHILE_CLICKING,
    ON_CLICK_EXIT,
    ON_BULLET_SHOOT,
    ON_BULLET_REFILL,

    // CONDITION
    CHECK_KEY_HELD,
    COMPARE_VALUE,
    COMPARE_VAR,
    CHECK_STATE,
    CHECK_TIMER,
    COMPARE_GLOBAL_FLOAT,
    CHECK_COLLIDED_STATE,

    // ACTION
    PRINT_MESSAGE,
    MOVE_OBJECT,
    ROTATE_OBJECT,
    SET_ROTATION,
    SET_ROTATION_RATE_OF_CHANGE,
    //ROTATE_TO,
    SET_POSITION,
    SET_SCALE,
	SCALE_PROPORTIONAL,
    SET_FLOAT,
    MODIFY_FLOAT,
    PLAY_SOUND,
    SET_STATE,
    RESET_TIMER,
    SET_TEXTURE,
	SET_POSITION_ON_COLLIDED, // might not work on those object that has input
    SET_STATE_ON_COLLIDED,
    APPLY_FORCE_ON_COLLIDED,
    SET_GLOBAL_FLOAT,
    MODIFY_GLOBAL_FLOAT,
    TRIGGER_BULLET_REFILL_EVENT, // whenever bullet is refilled, add this node
    RESTART_LEVEL,
    LOAD_LEVEL,
	DISABLE_COLLISION,

    // DEFAULT
    DEFAULT // nothing happen :)
};

struct LogicNode {
    int id = 0;
    NodeType type = NodeType::DEFAULT;
    //std::string name;
    NodeImplementation imp = NodeImplementation::DEFAULT;
    Vector2D position = Vector2D(0,0);
    bool hasInput = true;
    bool hasOutput = true;

    bool hasTrueOutput = false;
    bool hasFalseOutput = false;

    // for node configuration
    std::map<std::string, std::string> parameters;

    inline bool operator==(const LogicNode& other) const {
        return id == other.id && position.x == other.position.x &&
            position.y == other.position.y && parameters == other.parameters;
    }
};

class GameObject; // annoying forward declaration

struct VisualLC : public Component {
public:
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<VisualLC>(*this);
    }

    std::string filename; // name of the logic asset

    // editor data
	std::vector<LogicNode> nodes;
	std::vector<LogicConnection> connections;

	// runtime data
	bool justStarted = true;
    bool isExecuting = false;
    int currentExecutingNodeId = -1;
	std::map<std::string, int> intVar;
    std::map<std::string, float> floatVar;
    std::map<std::string, std::string> stringVar;
    std::map<std::string, bool> boolVar;
    std::map<std::string, float> timers;
    std::unordered_set<int> currentExePath;

	// global variable that shared across all objects
	static std::map<std::string, float> globalFloatVar;

	bool wasHoveredLastFrame = false; // for hover event detection
    bool wasClickedLastFrame = false; // on click event detection

    // for on hover events (caching only)
    bool hasHoverNodes = false;
	bool hoverNodesChecked = false;

    // for on click events (caching only)
    bool hasClickNodes = false;
    bool clickNodesChecked = false;

    // collision 
    std::unordered_set<GameObject*> curCollision;
    std::unordered_set<GameObject*> preCollision;
	GameObject* lastCollidedObj = nullptr;

    // bullet event
    bool bulletShot = false;
    bool bulletRefill = false;
};

// WaterBody component for creating animated water surfaces     
struct WaterBody : public Component {
public:
    WaterBody() = default;

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<WaterBody>(*this);
    }

    // Wave animation parameters
    float waveAmplitude = 0.2f;      
    float waveFrequency = 2.0f;       
    float waveSpeed = 1.0f;           
    float waveTime = 0.0f;            

    // Water visual properties
    glm::vec4 waterColor = glm::vec4(0.2f, 0.5f, 0.8f, 0.7f); 
    float waterAlpha = 0.7f;          

    // Subdivision for wave mesh
    int subdivisions = 20;            // segments waves

    // Water body dimensions
    float width = 10.0f;
    float height = 5.0f;

    // Physics properties
    bool isTrigger = true;            // If objects can pass through
    float buoyancyForce = 50.0f;      // Upward force applied to objects in water
    float waterDrag = 0.95f;          // Velocity damping when in water

    // Rendering flags
    bool renderWaves = true;
    bool useTexture = false;          
    GLuint waterTexture = 0;          
};

struct WeaponComponent : public Component {
public:
    WeaponComponent() = default;

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<WeaponComponent>(*this);
    }

    Vector2D barrelOffset{ 1.0f, 0.0f };

    bool aimAtMouse = true;

    // run-time data
    float baseLocalOffsetX = 0.0f;
    bool  baseLocalOffsetInitialised = false;
    float currentAngle = 0.0f;
};