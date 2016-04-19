#include "game.h"

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

    void renderplayer(gameent *d, const playermodelinfo &mdl, int color, float fade, int flags = 0, bool mainpass = true)
    {
        int lastaction = d->lastaction, anim = ANIM_IDLE|ANIM_LOOP, attack = 0, delay = 0;
        if(d->lastattack >= 0)
        {
            attack = attacks[d->lastattack].anim;
            delay = attacks[d->lastattack].attackdelay+50;
        }
        modelattach a[5];
        int ai = 0;
        if(weaps[d->weapselect].model)
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].weap==d->weapselect && lastmillis < lastaction + delay)
            {
                vanim = attacks[d->lastattack].vwepanim;
                vtime = lastaction;
            }
            defformatstring(weapname, "%s/vwep", mdl.model);
            a[ai++] = modelattach("tag_weapon", weapname, vanim, vtime);
        }
        if(mainpass && !(flags&MDL_ONLYSHADOW))
        {
            d->muzzle = vec(-1, -1, -1);
            if(weaps[d->weapselect].model) a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        }
        const char *mdlname = mdl.model;
        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch, roll = d->roll;
        vec o = d->feetpos();
        int basetime = 0, basetime2 = 0;
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

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX)
            {
                anim >>= ANIM_SECONDARY;
                swap(basetime, basetime2);
            }
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
        if(d != player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type == ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        rendermodel(mdlname, anim, o, yaw, pitch, roll, flags, d, a[0].tag ? a : NULL, basetime, basetime2, fade, vec4(vec::hexcolor(color), d->state == CS_LAGGED ? 0.5f : 1.0f));
    }

    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0)
    {
        renderplayer(d, getplayermodelinfo(d), getplayercolor(d), fade, flags);
    }

    void rendergame()
    {
        ai::render();

        bool third = isthirdperson();
        gameent *f = followingplayer(), *exclude = third ? NULL : f;
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || d->state==CS_SPECTATOR || d->state==CS_SPAWNING || d->lifesequence < 0 || d == exclude || (d->state==CS_DEAD && hidedead)) continue;
            renderplayer(d);
            copystring(d->info, colorname(d));
            if(d->state!=CS_DEAD)
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
        else if(!f && (player1->state==CS_ALIVE || (player1->state==CS_EDITING && third) || (player1->state==CS_DEAD && !hidedead)))
            renderplayer(player1, 1, third ? 0 : MDL_ONLYSHADOW);
        entities::renderentities();
        renderbouncers();
        renderprojectiles();
    }

    VARP(hudweap, 0, 1, 1);
    VARP(hudweapsway, 0, 1, 1);

    FVAR(swaystep, 1, 35.0f, 100);
    FVAR(swayside, 0, 0.10f, 1);
    FVAR(swayup, -1, 0.15f, 1);

    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0);

    void swayhudweap(int curtime)
    {
        gameent *d = hudplayer();
        if(d->state != CS_SPECTATOR)
        {
            if(d->physstate >= PHYS_SLOPE)
            {
                swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), d->maxspeed);
                swaydist += swayspeed*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade = 1;
            }
            else if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade -= 0.5f*(curtime*d->maxspeed)/(swaystep*1000.0f);
            }

            float k = pow(0.7f, curtime/10.0f);
            swaydir.mul(k);
            vec vel(d->vel);
            vel.add(d->falling);
            swaydir.add(vec(vel).mul((1-k)/(15*max(vel.magnitude(), d->maxspeed))));
        }
    }

    struct hudent : dynent
    {
        hudent() { type = ENT_CAMERA; }
    } weapinterp;

    void drawhudweap()
    {
        gameent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_EDITING || !hudweap || editmode)
        {
            d->muzzle = player1->muzzle = vec(-1, -1, -1);
            return;
        }
        vec sway;
        vecfromyawpitch(d->yaw, 0, 0, 1, sway);
        float steps = swaydist/swaystep*M_PI;
        sway.mul(swayside*cosf(steps));
        sway.z = swayup*(fabs(sinf(steps)) - 1);
        sway.add(swaydir).add(d->o);
        if(!hudweapsway) sway = d->o;
        d->muzzle = vec(-1, -1, -1);

        const playermodelinfo &mdl = getplayermodelinfo(d);
        int color = getplayercolor(d);
        defformatstring(mdlname, "%s/hwep", mdl.model);
        modelattach a[2];
        int ai = 0, lastaction = d->lastaction, anim = ANIM_WEAP_IDLE|ANIM_LOOP, basetime = 0;
        if(weaps[d->weapselect].model)
        {
            if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].weap==d->weapselect && lastmillis < lastaction+attacks[d->lastattack].attackdelay+50)
            {
                anim = attacks[d->lastattack].hudanim;
                basetime = lastaction;
            }
            defformatstring(weapname, "%s/hwep", mdl.model);
            a[ai++] = modelattach("tag_weapon", weapname, anim, basetime);
            a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        }
        else a[ai++] = modelattach("taw_weapon", &d->muzzle);
        rendermodel(mdlname, anim, sway, d->yaw, d->pitch, d->roll, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), 1));
        if(d->muzzle.x >= 0) d->muzzle = calcavatarpos(d->muzzle, 12);
    }

    void renderavatar()
    {
        drawhudweap();
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
        if(hudweap)
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
        }
    }

    void preloadsounds()
    {
        for(int i = S_JUMP; i <= S_DIE2; i++) preloadsound(i);
    }

    void preload()
    {
        if(hudweap) preloadweapons();
        preloadbouncers();
        preloadplayermodel();
        preloadsounds();
        entities::preloadentities();
    }

}

