#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"

// animations

enum
{
    ANIM_DEAD = ANIM_GAMESPECIFIC, ANIM_DYING, ANIM_IDLE,
    ANIM_FORWARD, ANIM_BACKWARD, ANIM_LEFT, ANIM_RIGHT,
    ANIM_RUN_FORWARD, ANIM_RUN_BACKWARD, ANIM_RUN_LEFT, ANIM_RUN_RIGHT,
    ANIM_JUMP_FORWARD, ANIM_JUMP_BACKWARD, ANIM_JUMP_LEFT, ANIM_JUMP_RIGHT, ANIM_JUMP,
    ANIM_CROUCH, ANIM_CRAWL_FORWARD, ANIM_CRAWL_BACKWARD, ANIM_CRAWL_LEFT, ANIM_CRAWL_RIGHT,
    ANIM_CROUCH_JUMP_FORWARD, ANIM_CROUCH_JUMP_BACKWARD, ANIM_CROUCH_JUMP_LEFT, ANIM_CROUCH_JUMP_RIGHT, ANIM_CROUCH_JUMP,
    ANIM_WALL_RUN_LEFT, ANIM_WALL_RUN_RIGHT, ANIM_WALL_JUMP, ANIM_SLIDE,
    ANIM_SWIM, ANIM_SINK, ANIM_SHOOT, ANIM_MELEE, ANIM_PAIN, ANIM_EDIT,
    ANIM_WEAP_IDLE, ANIM_WEAP_SHOOT, ANIM_WEAP_MELEE,
    ANIM_VWEP_IDLE, ANIM_VWEP_SHOOT, ANIM_VWEP_MELEE,
    NUMANIMS
};

static const char * const animnames[] =
{
    "mapmodel", "trigger on", "trigger off", "dead", "dying", "idle",
    "forward", "backward", "left", "right",
    "run forward", "run backward", "run left", "run right",
    "jump forward", "jump backward", "jump left", "jump right", "jump",
    "crouch", "crawl forward", "crawl backward", "crawl left", "crawl right",
    "crouch jump forward", "crouch jump backward", "crouch jump left", "crouch jump right", "crouch jump",
    "wall run left", "wall run right", "wall jump", "slide",
    "swim", "sink", "shoot", "melee", "pain", "edit",
    "weap idle", "weap shoot", "weap melee",
    "vwep idle", "vwep shoot", "vwep melee"
};

// console message types

enum
{
    CON_CHAT       = 1<<8,
    CON_GAMEINFO   = 1<<9,
    CON_FRAG_SELF  = 1<<10,
};

// network quantization scale
#define DMF 16.0f               // for world locations
#define DNF 100.0f              // for normalized vectors
#define DVELF 1.0f              // for playerspeed based velocity vectors

enum                            // static entity types
{
    NOTUSED = ET_EMPTY,         // entity slot not in use in map
    LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,     // attr1 = idx, attr2 = yaw, attr3 = pitch, attr4 = roll, attr5 = scale
    PLAYERSTART,                // attr1 = angle, attr2 = ai type
    ENVMAP = ET_ENVMAP,         // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
    DECAL = ET_DECAL,
    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = angle, attr2 = idx
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    PLACEHOLDER,
    MAXENTTYPES,

    I_FIRST = 0,
    I_LAST = -1
};

struct gameentity : extentity
{
};

enum { WEAP_NONE = 0, WEAP_RAIL, WEAP_PULSE, NUMWEAPS };
enum { ACT_IDLE = 0, ACT_SHOOT, ACT_MELEE, NUMACTS };
enum { ATK_NONE_GRAB = 0, ATK_NONE_MELEE, ATK_RAIL_SHOOT, ATK_RAIL_MELEE, ATK_PULSE_SHOOT, ATK_PULSE_MELEE, NUMATKS };

#define validweap(n) ((n) >= 0 && (n) < NUMWEAPS)
#define validact(n) ((n) >= 0 && (n) < NUMACTS)
#define validatk(n) ((n) >= 0 && (n) < NUMATKS)

enum
{
    M_EDIT       = 1<<3,
    M_DEMO       = 1<<4,
    M_LOCAL      = 1<<5,
    M_LOBBY      = 1<<6,
};

static struct gamemodeinfo
{
    const char *name, *prettyname;
    int flags;
    const char *info;
} gamemodes[] =
{
    { "demo", "Demo", M_DEMO | M_LOCAL, NULL},
    { "edit", "Edit", M_EDIT, "Cooperative Editing:\nEdit maps with multiple players simultaneously." },
    { "lobby", "Lobby", M_LOBBY, "Multiplayer Lobby:\nTest and mess around on maps with multiple players simultaneously." },
    { "story", "Story", M_LOCAL, "Story:\n Play through the Red Eclipse Horizon story." },
    { "scenario", "Scenario", M_LOCAL, "Scenario:\n Play through a self contained map scenario." }
};

#define STARTGAMEMODE (-1)
#define NUMGAMEMODES ((int)(sizeof(gamemodes)/sizeof(gamemodes[0])))

#define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
#define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))

#define m_demo         (m_check(gamemode, M_DEMO))
#define m_edit         (m_check(gamemode, M_EDIT))
#define m_lobby        (m_check(gamemode, M_LOBBY))
#define m_botmode      (m_checknot(gamemode, M_DEMO|M_LOCAL))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH, MM_INVALID = MM_START - 1 };

static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// hardcoded sounds, defined in sounds.cfg
enum
{
    S_JUMP = 0, S_LAND,
    S_SPLASHIN, S_SPLASHOUT, S_BURN,
    S_ITEMSPAWN, S_TELEPORT, S_JUMPPAD,
    S_MELEE, S_PULSE1, S_PULSE2, S_PULSEEXPLODE, S_RAIL1, S_RAIL2,
    S_WEAPLOAD, S_NOAMMO, S_HIT,
    S_PAIN1, S_PAIN2, S_DIE1, S_DIE2,
};

// network messages codes, c2s, c2c, s2c

enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

enum
{
    N_CONNECT = 0, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS,
    N_SHOOT, N_EXPLODE, N_SUICIDE,
    N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX,
    N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH,
    N_WEAPSELECT,
    N_MAPCHANGE, N_MAPVOTE, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD,
    N_PING, N_PONG, N_CLIENTPING,
    N_INTERMISSION, N_FORCEINTERMISSION,
    N_SERVMSG, N_ITEMLIST, N_RESUME,
    N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_CALCLIGHT, N_REMIP, N_EDITVSLOT, N_UNDO, N_REDO, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR,
    N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER,
    N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO,
    N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS,
    N_CLIENT,
    N_AUTHTRY, N_AUTHKICK, N_AUTHCHAL, N_AUTHANS, N_REQAUTH,
    N_PAUSEGAME, N_GAMESPEED,
    N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT,
    N_MAPCRC, N_CHECKMAPS,
    N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHCOLOR,
    N_SERVCMD,
    N_DEMOPACKET,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1,
    N_DIED, 4, N_DAMAGE, 5, N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 8, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_WEAPSELECT, 2, N_MAPCHANGE, 0, N_MAPVOTE, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 3,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_INTERMISSION, 1, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 11, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_CALCLIGHT, 1, N_REMIP, 1, N_EDITVSLOT, 16, N_UNDO, 0, N_REDO, 0, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 0, N_CLEARBANS, 1, N_CURRENTMASTER, 0, N_SPECTATOR, 3, N_SETMASTER, 0,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 2, N_SENDDEMO, 0,
    N_DEMOPLAYBACK, 3, N_RECORDDEMO, 2, N_STOPDEMO, 1, N_CLEARDEMOS, 2,
    N_CLIENT, 0,
    N_AUTHTRY, 0, N_AUTHKICK, 0, N_AUTHCHAL, 0, N_AUTHANS, 0, N_REQAUTH, 0,
    N_PAUSEGAME, 0, N_GAMESPEED, 0,
    N_ADDBOT, 2, N_DELBOT, 1, N_INITAI, 0, N_FROMAI, 2, N_BOTLIMIT, 2,
    N_MAPCRC, 0, N_CHECKMAPS, 1,
    N_SWITCHNAME, 0, N_SWITCHMODEL, 2, N_SWITCHCOLOR, 2,
    N_SERVCMD, 0,
    N_DEMOPACKET, 0,
    -1
};

#define HORIZON_SERVER_PORT 42000
#define HORIZON_LANINFO_PORT 41998
#define HORIZON_MASTER_PORT 41999
#define PROTOCOL_VERSION 2              // bump when protocol changes
#define DEMO_VERSION 1                  // bump when demo format changes
#define DEMO_MAGIC "HORIZON_DEMO\0\0"

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define MAXNAMELEN 15

enum
{
    HICON_X       = 20,
    HICON_Y       = 1650,
    HICON_TEXTY   = 1644,
    HICON_STEP    = 490,
    HICON_SIZE    = 120,
    HICON_SPACE   = 40
};

#if 0
static struct itemstat { int add, max, sound; const char *name; int icon, info; } itemstats[] =
{
};
#endif

#define validitem(n) false

#define MAXRAYS 1
#define EXP_SELFDAMDIV 2
#define EXP_SELFPUSH 2.5f
#define EXP_DISTSCALE 0.5f

static const struct attackinfo { int weap, action, anim, vwepanim, hudanim, sound, hudsound, attackdelay, damage, spread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    { WEAP_NONE,  ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_WEAP_MELEE, S_MELEE,  S_MELEE,  500, 20,  0, 2,    0,  0,   14, 1,    0,  0, 0, 0 },
    { WEAP_NONE,  ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_WEAP_MELEE, S_MELEE,  S_MELEE,  500, 20,  0, 2,    0,  0,   14, 1,    0,  0, 0, 0 },
    { WEAP_RAIL,  ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_WEAP_SHOOT, S_RAIL1,  S_RAIL2, 1300, 75,  0, 0,    0, 30, 2048, 1, 5000,  0, 0, 0 },
    { WEAP_RAIL,  ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_WEAP_MELEE, S_MELEE,  S_MELEE,  500, 20,  0, 2,    0,  0,   14, 1,    0,  0, 0, 0 },
    { WEAP_PULSE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_WEAP_SHOOT, S_PULSE1, S_PULSE2, 700, 40,  1, 1, 1000, 30, 1024, 1, 5000, 15, 0, 0 },
    { WEAP_PULSE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_WEAP_MELEE, S_MELEE,  S_MELEE,  500, 20,  0, 2,    0,  0,   14, 1,    0,  0, 0, 0 }
};

static const struct weapinfo { const char *name, *model; int attacks[NUMACTS]; } weaps[NUMWEAPS] =
{
    { "none", NULL, { -1, ATK_NONE_GRAB, ATK_NONE_MELEE }, },
    { "railgun", NULL, { -1, ATK_RAIL_SHOOT, ATK_RAIL_MELEE }, },
    { "pulse rifle", NULL, { -1, ATK_PULSE_SHOOT, ATK_PULSE_MELEE } }
};

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
    int health, maxhealth;
    int weapselect, weapwait;
    int ammo[NUMWEAPS];
    int aitype, skill;

    gamestate() : maxhealth(100), aitype(AI_NONE), skill(0) {}

    bool canpickup(int type)
    {
        return validitem(type);
    }

    void pickup(int type)
    {
    }

    void respawn()
    {
        health = maxhealth;
        weapselect = WEAP_NONE;
        weapwait = 0;
        loopi(NUMWEAPS) ammo[i] = 0;
    }

    void spawnstate(int gamemode)
    {
        loopi(NUMWEAPS) ammo[i] = 0;
        weapselect = WEAP_NONE;
        ammo[WEAP_NONE] = 1;
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        health -= damage;
        return damage;
    }

    int hasammo(int weap, int exclude = -1)
    {
        return validweap(weap) && weap != exclude && ammo[weap] > 0;
    }
};

struct gameent : dynent, gamestate
{
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastattack;
    int attacking;
    int lastpickup, lastpickupmillis, flagpickup;
    int frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    string name, info;
    int playermodel, playercolor;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle, toe[2];

    gameent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), playermodel(-1), playercolor(0), ai(NULL), ownernum(-1)
    {
        name[0] = info[0] = 0;
        muzzle = toe[0] = toe[1] = vec(-1, -1, -1);
        respawn();
    }
    ~gameent()
    {
        freeeditinfo(edit);
        if(ai) delete ai;
    }

    void hitpush(int damage, const vec &dir, gameent *actor, int atk)
    {
        vec push(dir);
        push.mul((actor==this && attacks[atk].exprad ? EXP_SELFPUSH : 1.0f)*attacks[atk].hitpush*damage/weight);
        vel.add(push);
    }

    void respawn()
    {
        dynent::reset();
        gamestate::respawn();
        respawned = suicided = -1;
        lastaction = 0;
        lastattack = -1;
        attacking = ACT_IDLE;
        lastpickup = -1;
        lastpickupmillis = 0;
        flagpickup = 0;
        lastnode = -1;
    }

    void startgame()
    {
        frags = flags = deaths = 0;
        totaldamage = totalshots = 0;
        lifesequence = -1;
        respawned = suicided = -2;
    }
};

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void checkitems(gameent *d);
    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);
}

namespace game
{
    extern int gamemode;

    // game
    extern int nextmode;
    extern string clientmap;
    extern bool intermission;
    extern int maptime, maprealtime, maplimit;
    extern gameent *player1;
    extern vector<gameent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
    extern int following;
    extern int smoothmove, smoothdist;

    extern bool clientoption(const char *arg);
    extern gameent *getclient(int cn);
    extern gameent *newclient(int cn);
    extern const char *colorname(gameent *d, const char *name = NULL, const char *alt = NULL, const char *color = "");
    extern gameent *pointatplayer();
    extern gameent *hudplayer();
    extern gameent *followingplayer();
    extern void stopfollowing();
    extern void checkfollow();
    extern void nextfollow(int dir = 1);
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(gameent *);
    extern void deathstate(gameent *d, bool restore = false);
    extern void damaged(int damage, gameent *d, gameent *actor, bool local = true);
    extern void killed(gameent *d, gameent *actor);
    extern void startintermission();
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    // client
    extern bool connected, remote, demoplayback;
    extern string servdesc;
    extern vector<uchar> messages;

    extern int parseplayer(const char *arg);
    extern void ignore(int cn);
    extern void unignore(int cn);
    extern bool isignored(int cn);
    extern bool addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name);
    extern void switchplayermodel(int playermodel);
    extern void switchplayercolor(int playercolor);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void c2sinfo(bool force = false);
    extern void sendposition(gameent *d, bool reliable = false);

    // weapon
    extern int getweapon(const char *name);
    extern void shoot(gameent *d, const vec &targ);
    extern void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction);
    extern void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int dam, int atk);
    extern void explodeeffects(int atk, gameent *d, bool local, int id = 0);
    extern void damageeffect(int damage, gameent *d, bool thirdperson = true);
    extern void gibeffect(int damage, const vec &vel, gameent *d);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float margin = 0, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin = 0, float &dist = intersectdist);
    extern void clearbouncers();
    extern void updatebouncers(int curtime);
    extern void removebouncers(gameent *owner);
    extern void renderbouncers();
    extern void clearprojectiles();
    extern void updateprojectiles(int curtime);
    extern void removeprojectiles(gameent *owner);
    extern void renderprojectiles();
    extern void preloadbouncers();
    extern void removeweapons(gameent *owner);
    extern void updateweapons(int curtime);
    extern void weapselect(int weap, gameent *d);
    extern void weaponswitch(gameent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);

    // render
    struct playermodelinfo
    {
        const char *model, *icon;
        bool ragdoll;
    };

    extern void saveragdoll(gameent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern const playermodelinfo &getplayermodelinfo(gameent *d);
    extern int getplayercolor(gameent *d);
    extern int chooserandomplayermodel(int seed);
    extern void syncplayer();
    extern vec hudweaporigin(int weap, const vec &from, const vec &to, gameent *d);
    extern void resetsway();
    extern void addsway(gameent *d);
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *modeprettyname(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void stopdemo();
    extern void forcemap(const char *map, int mode);
    extern void forcepaused(bool paused);
    extern void forcegamespeed(int speed);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);
}

#endif

