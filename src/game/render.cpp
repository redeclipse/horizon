#include "game.h"
extern int hudw, hudh;
namespace game
{
    VARP(ragdoll, 0, 1, 1);
    VARP(ragdollmillis, 0, 10000, 300000);
    VARP(ragdollfade, 0, 100, 5000);
    VARP(forceplayermodels, 0, 0, 1);
    VARP(hidedead, 0, 0, 1);

    extern int playermodel;

    vector<gameent *> ragdolls;

    void saveragdoll(gameent *d)
    {
        if(!d->ragdoll || !ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        if(d==player1) r->playermodel = playermodel;
        ragdolls.add(r);
        d->ragdoll = NULL;
    }

    void clearragdolls()
    {
        ragdolls.deletecontents();
    }

    void moveragdolls()
    {
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(lastmillis > d->lastupdate + ragdollmillis)
            {
                delete ragdolls.remove(i--);
                continue;
            }
            moveragdoll(d);
        }
    }

    static const int playercolors[] =
    {
        0xA12020,
        0xA15B28,
        0xB39D52,
        0x3E752F,
        0x3F748C,
        0x214C85,
        0xB3668C,
        0x523678,
        0xB3ADA3
    };

    extern void changedplayercolor();
    VARFP(playercolor, 0, 4, sizeof(playercolors)/sizeof(playercolors[0])-1, changedplayercolor());

    static const playermodelinfo playermodels[] =
    {
        { "player/male", "player" , true },
        { "player/female", "player" , true }
    };

    extern void changedplayermodel();
    VARFP(playermodel, 0, 0, sizeof(playermodels)/sizeof(playermodels[0])-1, changedplayermodel());

    int chooserandomplayermodel(int seed)
    {
        return (seed&0xFFFF)%(sizeof(playermodels)/sizeof(playermodels[0]));
    }

    const playermodelinfo *getplayermodelinfo(int n)
    {
        if(size_t(n) >= sizeof(playermodels)/sizeof(playermodels[0])) return NULL;
        return &playermodels[n];
    }

    const playermodelinfo &getplayermodelinfo(gameent *d)
    {
        const playermodelinfo *mdl = getplayermodelinfo(d==player1 || forceplayermodels ? playermodel : d->playermodel);
        if(!mdl) mdl = getplayermodelinfo(playermodel);
        return *mdl;
    }

    int getplayercolor(int color)
    {
        return playercolors[color%(sizeof(playercolors)/sizeof(playercolors[0]))];
    }

    ICOMMAND(getplayercolor, "i", (int *color), intret(getplayercolor(*color)));

    int getplayercolor(gameent *d)
    {
        return getplayercolor((d->playercolor>>0)&0x1F);
    }

    void changedplayermodel()
    {
        if(player1->clientnum < 0) player1->playermodel = playermodel;
        if(player1->ragdoll) cleanragdoll(player1);
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(!d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || !d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
    }

    void changedplayercolor()
    {
        if(player1->clientnum < 0) player1->playercolor = playercolor;
    }

    void syncplayer()
    {
        if(player1->playermodel != playermodel)
        {
            player1->playermodel = playermodel;
            addmsg(N_SWITCHMODEL, "ri", player1->playermodel);
        }

        int col = playercolor;
        if(player1->playercolor != col)
        {
            player1->playercolor = col;
            addmsg(N_SWITCHCOLOR, "ri", player1->playercolor);
        }
    }

    void preloadplayermodel()
    {
        loopi(sizeof(playermodels)/sizeof(playermodels[0]))
        {
            const playermodelinfo *mdl = getplayermodelinfo(i);
            if(!mdl) break;
            if(i != playermodel && (!multiplayer(false) || forceplayermodels)) continue;
            preloadmodel(mdl->model);
        }
    }

    int numanims() { return NUMANIMS; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    VAR(animoverride, -1, 0, NUMANIMS-1);
    VAR(testanims, 0, 0, 1);
    VAR(testpitch, -90, 0, 90);

    VARP(firstpersonmodel, 0, 2, 2);

    FVAR(firstpersonspine, 0, 0.5f, 1);
    FVAR(firstpersonbodydist, -10, 0, 10);
    FVAR(firstpersonbodyside, -10, 0, 10);
    FVAR(firstpersonbodypitch, -1, 1, 1);
    FVAR(firstpersonbodyz, 0, 7, 10);

    FVAR(firstpersonpitchmin, 0, 90, 90);
    FVAR(firstpersonpitchmax, 0, 45, 90);
    FVAR(firstpersonpitchscale, -1, 1, 1);
    FVAR(firstpersonbodyfeet, -1, 4.75f, 20);

    vec thirdpos(const vec &pos, float yaw, float pitch, float dist, float side)
    {
        static struct tpcam : physent
        {
            tpcam()
            {
                physent::reset();
                type = ENT_CAMERA;
                state = CS_ALIVE;
                eyeheight = aboveeye = radius = xradius = yradius = 2;
            }
        } c;
        c.o = pos;
        if(dist || side)
        {
            vec dir[3] = { vec(0, 0, 0), vec(0, 0, 0), vec(0, 0, 0) };
            if(dist) vecfromyawpitch(yaw, pitch, -1, 0, dir[0]);
            if(side) vecfromyawpitch(yaw, pitch, 0, -1, dir[1]);
            dir[2] = dir[0].mul(dist).add(dir[1].mul(side)).normalize();
            movecamera(&c, dir[2], dist, 0.1f);
        }
        return c.o;
    }

    float swayfade = 0, swayspeed = 0, swaydist = 0, bobfade = 0, bobdist = 0;
    vec swaydir(0, 0, 0), swaypush(0, 0, 0);

    VAR(firstpersonsway, 0, 1, 1);
    FVAR(firstpersonswaymin, 0, 0.15f, 1);
    FVAR(firstpersonswaystep, 1, 28.f, 1000);
    FVAR(firstpersonswayside, 0, 0.05f, 10);
    FVAR(firstpersonswayup, 0, 0.06f, 10);

    VAR(firstpersonbob, 0, 1, 1);
    FVAR(firstpersonbobmin, 0, 0.2f, 1);
    FVAR(firstpersonbobstep, 1, 28.f, 1000);
    FVAR(firstpersonbobroll, 0, 0.35f, 10);
    FVAR(firstpersonbobside, 0, 0.75f, 10);
    FVAR(firstpersonbobup, 0, 0.75f, 10);
    FVAR(firstpersonbobtopspeed, 0, 75, 1000);
    FVAR(firstpersonbobfocusmindist, 0, 64, 10000);
    FVAR(firstpersonbobfocusmaxdist, 0, 256, 10000);
    FVAR(firstpersonbobfocus, 0, 0.5f, 1);

    void fixfullrange(float &yaw, float &pitch, float &roll, bool full)
    {
        if(full)
        {
            while(pitch < -180.0f) pitch += 360.0f;
            while(pitch >= 180.0f) pitch -= 360.0f;
            while(roll < -180.0f) roll += 360.0f;
            while(roll >= 180.0f) roll -= 360.0f;
        }
        else
        {
            if(pitch > 89.9f) pitch = 89.9f;
            if(pitch < -89.9f) pitch = -89.9f;
            if(roll > 89.9f) roll = 89.9f;
            if(roll < -89.9f) roll = -89.9f;
        }
        while(yaw < 0.0f) yaw += 360.0f;
        while(yaw >= 360.0f) yaw -= 360.0f;
    }

    void fixrange(float &yaw, float &pitch)
    {
        float r = 0.f;
        fixfullrange(yaw, pitch, r, false);
    }

    void calcangles(physent *c, dynent *d)
    {
        c->yaw = d->yaw;
        c->pitch = d->pitch;
        if(firstpersonbob && !intermission && d->state == CS_ALIVE && !isthirdperson())
        {
            float scale = 1;
            if(firstpersonbobtopspeed) scale *= clamp(d->vel.magnitude()/firstpersonbobtopspeed, firstpersonbobmin, 1.f);
            c->roll = d->roll;
            if(scale > 0)
            {
                vec dir(d->yaw, d->pitch);
                float steps = bobdist/firstpersonbobstep*M_PI, dist = raycube(c->o, dir, firstpersonbobfocusmaxdist, RAY_CLIPMAT|RAY_POLY), yaw, pitch;
                if(dist < 0 || dist > firstpersonbobfocusmaxdist) dist = firstpersonbobfocusmaxdist;
                else if(dist < firstpersonbobfocusmindist) dist = firstpersonbobfocusmindist;
                vectoyawpitch(vec(firstpersonbobside*cosf(steps), dist, firstpersonbobup*(fabs(sinf(steps)) - 1)), yaw, pitch);
                c->yaw -= yaw*firstpersonbobfocus*scale;
                c->pitch -= pitch*firstpersonbobfocus*scale;
                c->roll += (1-firstpersonbobfocus)*firstpersonbobroll*cosf(steps)*scale;
                fixfullrange(c->yaw, c->pitch, c->roll, false);
            }
        }
        else c->roll = 0;
    }

    float firstpersonspineoffset = 0;
    vec firstpos(physent *d, const vec &pos, float yaw, float pitch)
    {
        if(d->state != CS_ALIVE) return pos;
        static struct fpcam : physent
        {
            fpcam()
            {
                physent::reset();
                type = ENT_CAMERA;
                state = CS_ALIVE;
                eyeheight = aboveeye = radius = xradius = yradius = 2;
            }
        } c;

        vec to = pos;
        c.o = pos;
        if(firstpersonspine > 0)
        {
            float spineoff = firstpersonspine*d->eyeheight*0.5f;
            to.z -= spineoff;
            float lean = clamp(pitch, -firstpersonpitchmin, firstpersonpitchmax);
            if(firstpersonpitchscale >= 0) lean *= firstpersonpitchscale;
            to.add(vec(yaw*RAD, (lean+90)*RAD).mul(spineoff));
        }
        if(firstpersonbob && !intermission && d->state == CS_ALIVE)
        {
            float scale = 1;
            if(firstpersonbobtopspeed) scale *= clamp(d->vel.magnitude()/firstpersonbobtopspeed, firstpersonbobmin, 1.f);
            if(scale > 0)
            {
                float steps = bobdist/firstpersonbobstep*M_PI;
                vec dir = vec(yaw*RAD, 0.f).mul(firstpersonbobside*cosf(steps)*scale);
                dir.z = firstpersonbobup*(fabs(sinf(steps)) - 1)*scale;
                to.add(dir);
            }
        }
        c.o.z = to.z; // assume inside ourselves is safe
        vec dir = vec(to).sub(c.o), old = c.o;
        if(dir.iszero()) return c.o;
        float dist = dir.magnitude();
        dir.normalize();
        movecamera(&c, dir, dist, 0.1f);
        firstpersonspineoffset = max(dist-old.dist(c.o), 0.f);
        return c.o;
    }

    vec camerapos(physent *d, bool hasfoc, bool hasyp, float yaw, float pitch)
    {
        vec pos = d->o;
        if(d == hudplayer() || hasfoc)
        {
            if(!hasyp)
            {
                yaw = d->yaw;
                pitch = d->pitch;
            }
            if(isthirdperson()) pos = thirdpos(pos, yaw, pitch, 10, 0);
            else pos = firstpos(d, pos, yaw, pitch);
        }
        return pos;
    }

    void resetsway()
    {
        swaydir = swaypush = vec(0, 0, 0);
        swayfade = swayspeed = swaydist = bobfade = bobdist = 0;
    }

    void addsway(gameent *d)
    {
        float speed = d->maxspeed, step = firstpersonbob ? firstpersonbobstep : firstpersonswaystep;
        if(d->state == CS_ALIVE && (d->physstate >= PHYS_SLOPE || d->parkourside))
        {
            swayspeed = max(speed*firstpersonswaymin, min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), speed));
            swaydist += swayspeed*curtime/1000.0f;
            swaydist = fmod(swaydist, 2*step);
            bobdist += swayspeed*curtime/1000.0f;
            bobdist = fmod(bobdist, 2*firstpersonbobstep);
            bobfade = swayfade = 1;
        }
        else
        {
            if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*step);
                swayfade -= 0.5f*(curtime*speed)/(step*1000.0f);
            }
            if(bobfade > 0)
            {
                bobdist += swayspeed*bobfade*curtime/1000.0f;
                bobdist = fmod(bobdist, 2*firstpersonbobstep);
                bobfade -= 0.5f*(curtime*speed)/(firstpersonbobstep*1000.0f);
            }
        }

        float k = pow(0.7f, curtime/25.0f);
        swaydir.mul(k);
        vec inertia = vec(d->vel).add(d->falling);
        float speedscale = max(inertia.magnitude(), speed);
        if(d->state == CS_ALIVE && speedscale > 0) swaydir.add(vec(inertia).mul((1-k)/(15*speedscale)));
        swaypush.mul(pow(0.5f, curtime/25.0f));
    }

    struct avatarent : dynent
    {
        avatarent() { type = ENT_CAMERA; }
    };
    avatarent avatarmodel, bodymodel;

    void renderplayer(gameent *d, const playermodelinfo &mdl, int third, int color, float fade, int flags = 0, bool mainpass = true)
    {
        modelattach a[5];
        const char *mnames[3] = { "/hwep", "/vwep", "/body" };
        float yaw = third == 1 && testanims && d == player1 ? 0 : d->yaw,
              pitch = third == 1 && testpitch && d == player1 ? testpitch : d->pitch, roll = third ? 0.f : d->roll;
        int basetime = 0, basetime2 = 0, ai = 0, lastaction = d->lastaction, anim = ANIM_IDLE|ANIM_LOOP, attack = 0, delay = 0;
        vec o = third ? d->feetpos() : camerapos(d);
        if(third == 2)
        {
            o.sub(vec(yaw*RAD, 0.f).mul(firstpersonbodydist+firstpersonspineoffset));
            o.sub(vec(yaw*RAD, 0.f).rotate_around_z(90*RAD).mul(firstpersonbodyside));
            o.z -= firstpersonbodyz;
            if(firstpersonbodyfeet >= 0)
            {
                float minz = max(d->toe[0].z, d->toe[1].z)+(firstpersonbodyfeet);
                if(minz > camera1->o.z) o.z -= minz-camera1->o.z;
            }
        }
        else if(!intermission)
        {
            if(third == 1 && d == hudplayer() && d == player1 && isthirdperson())
                vectoyawpitch(vec(worldpos).sub(d->headpos()).normalize(), yaw, pitch);
            else if(!third && firstpersonsway)
            {
                float steps = swaydist/(firstpersonbob ? firstpersonbobstep : firstpersonswaystep)*M_PI;
                vec dir = vec(d->yaw*RAD, 0.f).mul(firstpersonswayside*cosf(steps));
                dir.z = firstpersonswayup*(fabs(sinf(steps)) - 1);
                o.add(dir).add(swaydir).add(swaypush);
            }
        }
        if(d->lastattack >= 0)
        {
            attack = attacks[d->lastattack].anim;
            delay = attacks[d->lastattack].attackdelay+50;
        }
        if(third != 2)
        {
            if(weaps[d->weapselect].model)
            {
                int vanim = (third ? ANIM_VWEP_IDLE : ANIM_WEAP_IDLE)|ANIM_LOOP, vtime = 0;
                if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].weap==d->weapselect && lastmillis < lastaction + delay)
                {
                    vanim = third ? attacks[d->lastattack].vwepanim : attacks[d->lastattack].hudanim;
                    vtime = lastaction;
                }
                defformatstring(weapname, "%s%s", mdl.model, mnames[third]);
                a[ai++] = modelattach("tag_weapon", weapname, vanim, vtime);
            }
            if(mainpass && !(flags&MDL_ONLYSHADOW))
            {
                d->muzzle = vec(-1, -1, -1);
                if(weaps[d->weapselect].model) a[ai++] = modelattach("tag_muzzle", &d->muzzle);
            }
        }
        if(third)
        {
            a[ai++] = modelattach("tag_ltoe", &d->toe[0]);
            a[ai++] = modelattach("tag_rtoe", &d->toe[1]);
        }

        defformatstring(mdlname, "%s%s", mdl.model, third != 1 ? mnames[third] : "");
        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
        else if(d->state==CS_DEAD)
        {
            anim = ANIM_DYING|ANIM_NOPITCH;
            basetime = d->lastpain;
            if(ragdoll && mdl.ragdoll) anim |= ANIM_RAGDOLL;
            else if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
        }
        else if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
        else if(!intermission && game::allowmove(d))
        {
            if(lastmillis-d->lastpain < 300)
            {
                anim = ANIM_PAIN;
                basetime = d->lastpain;
            }
            else if(d->lastpain < lastaction && lastmillis-lastaction < delay)
            {
                anim = attack;
                basetime = lastaction;
            }

            if(d->inwater && d->physstate<=PHYS_FALL) anim |= ((d->move || d->strafe || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->parkourside) anim |= ((d->parkourside>0 ? ANIM_WALL_RUN_LEFT : ANIM_WALL_RUN_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->sliding(lastmillis, SLIDETIME))
            {
                basetime2 = d->lastslide;
                anim |= (ANIM_SLIDE|ANIM_LOOP)<<ANIM_SECONDARY;
            }
            else if(d->physstate == PHYS_FALL && d->timeinair >= 50)
            {
                basetime2 = lastmillis-d->timeinair;
                bool jump = (d->lastjump && lastmillis-d->lastjump < JUMPDELAY/2), upwall = (d->lastupwall && lastmillis-d->lastupwall < JUMPDELAY);
                if(jump || upwall)
                {
                    anim |= (ANIM_WALL_JUMP|ANIM_LOOP)<<ANIM_SECONDARY;
                    basetime2 = jump ? d->lastjump : d->lastupwall;
                }
                if(d->crouching)
                {
                    if(d->strafe) anim |= (d->strafe>0 ? ANIM_CROUCH_JUMP_LEFT : ANIM_CROUCH_JUMP_RIGHT)<<ANIM_SECONDARY;
                    else if(d->move>0) anim |= ANIM_CROUCH_JUMP_FORWARD<<ANIM_SECONDARY;
                    else if(d->move<0) anim |= ANIM_CROUCH_JUMP_BACKWARD<<ANIM_SECONDARY;
                    else anim |= ANIM_CROUCH_JUMP<<ANIM_SECONDARY;
                }
                else if(d->strafe) anim |= (d->strafe>0 ? ANIM_JUMP_LEFT : ANIM_JUMP_RIGHT)<<ANIM_SECONDARY;
                else if(d->move>0) anim |= ANIM_JUMP_FORWARD<<ANIM_SECONDARY;
                else if(d->move<0) anim |= ANIM_JUMP_BACKWARD<<ANIM_SECONDARY;
                else anim |= ANIM_JUMP<<ANIM_SECONDARY;
                if(!basetime2) anim |= ANIM_END<<ANIM_SECONDARY;
            }
            else if(d->crouching)
            {
                if(d->strafe) anim |= ((d->strafe>0 ? ANIM_CRAWL_LEFT : ANIM_CRAWL_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->move>0) anim |= (ANIM_CRAWL_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->move<0) anim |= (ANIM_CRAWL_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                else anim |= (ANIM_CROUCH|ANIM_LOOP)<<ANIM_SECONDARY;
            }
            else if(d->velxychk(RUNSPEED))
            {
                if(d->strafe) anim |= ((d->strafe>0 ? ANIM_RUN_LEFT : ANIM_RUN_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->move>0) anim |= (ANIM_RUN_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
                else if(d->move<0) anim |= (ANIM_RUN_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;
            }
            else if(d->strafe) anim |= ((d->strafe>0 ? ANIM_LEFT : ANIM_RIGHT)|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->move>0) anim |= (ANIM_FORWARD|ANIM_LOOP)<<ANIM_SECONDARY;
            else if(d->move<0) anim |= (ANIM_BACKWARD|ANIM_LOOP)<<ANIM_SECONDARY;

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
        if(d != player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type != ENT_PLAYER) flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        dynent *e = third ? (third != 2 ? (dynent *)d : (dynent *)&bodymodel) : (dynent *)&avatarmodel;
        rendermodel(mdlname, anim, o, yaw, third == 2 ? pitch*firstpersonbodypitch : pitch, third == 2 ? 0.f : roll, flags, e, a[0].tag ? a : NULL, basetime, basetime2, fade, vec4(vec::hexcolor(color), d->state == CS_LAGGED ? 0.5f : 1.0f));
    }

    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0)
    {
        renderplayer(d, getplayermodelinfo(d), 1, getplayercolor(d), fade, flags);
    }

    void rendergame()
    {
        ai::render();

        bool third = isthirdperson();
        gameent *f = followingplayer(), *exclude = third ? NULL : f;
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || d->state == CS_SPECTATOR || d->state == CS_SPAWNING || d->lifesequence < 0 || d == exclude || (d->state == CS_DEAD && hidedead)) continue;
            renderplayer(d);
            copystring(d->info, colorname(d));
            if(d->state != CS_DEAD)
                particle_text(d->abovehead(), d->info, PART_TEXT, 1, 0xFFFFFF, 2.0f);
        }
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            float fade = 1.0f;
            if(ragdollmillis && ragdollfade)
                fade -= clamp(float(lastmillis - (d->lastupdate + max(ragdollmillis - ragdollfade, 0)))/min(ragdollmillis, ragdollfade), 0.0f, 1.0f);
            renderplayer(d, fade);
        }
        if(exclude)
            renderplayer(exclude, 1, MDL_ONLYSHADOW);
        else if(!f && (player1->state == CS_ALIVE || (player1->state == CS_EDITING && third) || (player1->state==CS_DEAD && !hidedead)))
            renderplayer(player1, 1, third ? 0 : MDL_ONLYSHADOW);
        entities::renderentities();
        renderbouncers();
        renderprojectiles();
    }

    void renderavatar()
    {
        if(player1->state == CS_EDITING) return;
        gameent *d = hudplayer();
        if(firstpersonmodel) renderplayer(d, getplayermodelinfo(d), 0, getplayercolor(d), 1, MDL_NOBATCH);
        if(firstpersonmodel == 2) renderplayer(d, getplayermodelinfo(d), 2, getplayercolor(d), 1, MDL_NOBATCH, false);
        if(d->muzzle.x >= 0) d->muzzle = calcavatarpos(d->muzzle, 12);
   }

    void renderplayerpreview(int model, int color, int weap)
    {
        static gameent *previewent = NULL;
        if(!previewent)
        {
            previewent = new gameent;
            loopi(NUMWEAPS) previewent->ammo[i] = 1;
        }
        float height = previewent->eyeheight + previewent->aboveeye,
              zrad = height/2;
        vec2 xyrad = vec2(previewent->xradius, previewent->yradius).max(height/4);
        previewent->o = calcmodelpreviewpos(vec(xyrad, zrad), previewent->yaw).addz(previewent->eyeheight - zrad);
        previewent->weapselect = validweap(weap) ? weap : WEAP_RAIL;
        const playermodelinfo *mdlinfo = getplayermodelinfo(model);
        if(!mdlinfo) return;
        renderplayer(previewent, *mdlinfo, getplayercolor(color), 1, 0, false);
    }

    vec hudweaporigin(int weap, const vec &from, const vec &to, gameent *d)
    {
        if(d->muzzle.x >= 0) return d->muzzle;
        vec offset(from);
        if(d!=hudplayer() || isthirdperson())
        {
            vec front, right;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, front);
            offset.add(front.mul(d->radius));
            offset.z += (d->aboveeye + d->eyeheight)*0.75f - d->eyeheight;
            vecfromyawpitch(d->yaw, 0, 0, -1, right);
            offset.add(right.mul(0.5f*d->radius));
            offset.add(front);
            return offset;
        }
        offset.add(vec(to).sub(from).normalize().mul(2));
        if(firstpersonmodel)
        {
            offset.sub(vec(camup).mul(1.0f));
            offset.add(vec(camright).mul(0.8f));
        }
        else offset.sub(vec(camup).mul(0.8f));
        return offset;
    }

    void preloadweapons()
    {
        loopi(NUMWEAPS)
        {
            const char *file = weaps[i].model;
            if(!file || !*file) continue;
            string fname;
            formatstring(fname, "%s/hwep", file);
            preloadmodel(fname);
            formatstring(fname, "%s/vwep", file);
            preloadmodel(fname);
            formatstring(fname, "%s/body", file);
            preloadmodel(fname);
        }
    }

    void preloadsounds()
    {
        for(int i = S_JUMP; i <= S_DIE2; i++) preloadsound(i);
    }

    void preload()
    {
        if(firstpersonmodel) preloadweapons();
        preloadbouncers();
        preloadplayermodel();
        preloadsounds();
        entities::preloadentities();
    }

}
