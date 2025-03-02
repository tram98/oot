/*
 * File: z_bg_mori_hashigo.c
 * Overlay: ovl_Bg_Mori_Hashigo
 * Description: Falling ladder and clasp that holds it. Unused.
 */

#include "z_bg_mori_hashigo.h"
#include "objects/object_mori_objects/object_mori_objects.h"

#define FLAGS 0

void BgMoriHashigo_Init(Actor* thisx, GlobalContext* globalCtx);
void BgMoriHashigo_Destroy(Actor* thisx, GlobalContext* globalCtx);
void BgMoriHashigo_Update(Actor* thisx, GlobalContext* globalCtx);
void BgMoriHashigo_Draw(Actor* thisx, GlobalContext* globalCtx);

void BgMoriHashigo_SetupWaitForMoriTex(BgMoriHashigo* this);
void BgMoriHashigo_WaitForMoriTex(BgMoriHashigo* this, GlobalContext* globalCtx);
void BgMoriHashigo_SetupClasp(BgMoriHashigo* this);
void BgMoriHashigo_Clasp(BgMoriHashigo* this, GlobalContext* globalCtx);
void BgMoriHashigo_SetupLadderWait(BgMoriHashigo* this);
void BgMoriHashigo_LadderWait(BgMoriHashigo* this, GlobalContext* globalCtx);
void BgMoriHashigo_SetupLadderFall(BgMoriHashigo* this);
void BgMoriHashigo_LadderFall(BgMoriHashigo* this, GlobalContext* globalCtx);
void BgMoriHashigo_SetupLadderRest(BgMoriHashigo* this);

const ActorInit Bg_Mori_Hashigo_InitVars = {
    ACTOR_BG_MORI_HASHIGO,
    ACTORCAT_BG,
    FLAGS,
    OBJECT_MORI_OBJECTS,
    sizeof(BgMoriHashigo),
    (ActorFunc)BgMoriHashigo_Init,
    (ActorFunc)BgMoriHashigo_Destroy,
    (ActorFunc)BgMoriHashigo_Update,
    NULL,
};

static ColliderJntSphElementInit sJntSphElementsInit[1] = {
    {
        {
            ELEMTYPE_UNK4,
            { 0x00000000, 0x00, 0x00 },
            { 0x0001F820, 0x00, 0x00 },
            TOUCH_NONE,
            BUMP_ON,
            OCELEM_NONE,
        },
        { 0, { { 0, 0, 0 }, 25 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static InitChainEntry sInitChainClasp[] = {
    ICHAIN_F32(uncullZoneForward, 1000, ICHAIN_CONTINUE),  ICHAIN_F32(uncullZoneScale, 400, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 1000, ICHAIN_CONTINUE), ICHAIN_U8(targetMode, 3, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 40, ICHAIN_CONTINUE),    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

static InitChainEntry sInitChainLadder[] = {
    ICHAIN_F32(uncullZoneForward, 1000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 400, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 1000, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 1000, ICHAIN_STOP),
};

void BgMoriHashigo_InitDynapoly(BgMoriHashigo* this, GlobalContext* globalCtx, CollisionHeader* collision,
                                s32 moveFlag) {
    s32 pad;
    CollisionHeader* colHeader;
    s32 pad2;

    colHeader = NULL;
    DynaPolyActor_Init(&this->dyna, moveFlag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(globalCtx, &globalCtx->colCtx.dyna, &this->dyna.actor, colHeader);

    if (this->dyna.bgId == BG_ACTOR_MAX) {
        // "Warning : move BG login failed"
        osSyncPrintf("Warning : move BG 登録失敗(%s %d)(name %d)(arg_data 0x%04x)\n", "../z_bg_mori_hashigo.c", 164,
                     this->dyna.actor.id, this->dyna.actor.params);
    }
}

void BgMoriHashigo_InitCollider(BgMoriHashigo* this, GlobalContext* globalCtx) {
    s32 pad;

    Collider_InitJntSph(globalCtx, &this->collider);
    Collider_SetJntSph(globalCtx, &this->collider, &this->dyna.actor, &sJntSphInit, this->colliderItems);

    this->collider.elements[0].dim.worldSphere.center.x = (s16)this->dyna.actor.world.pos.x;
    this->collider.elements[0].dim.worldSphere.center.y = (s16)this->dyna.actor.world.pos.y + 21;
    this->collider.elements[0].dim.worldSphere.center.z = (s16)this->dyna.actor.world.pos.z;
    this->collider.elements[0].dim.worldSphere.radius = 19;
}

s32 BgMoriHashigo_SpawnLadder(BgMoriHashigo* this, GlobalContext* globalCtx) {
    f32 sn;
    f32 cs;
    Vec3f pos;
    Actor* ladder;

    cs = Math_CosS(this->dyna.actor.shape.rot.y);
    sn = Math_SinS(this->dyna.actor.shape.rot.y);

    pos.x = 6.0f * sn + this->dyna.actor.world.pos.x;
    pos.y = -210.0f + this->dyna.actor.world.pos.y;
    pos.z = 6.0f * cs + this->dyna.actor.world.pos.z;

    ladder = Actor_SpawnAsChild(&globalCtx->actorCtx, &this->dyna.actor, globalCtx, ACTOR_BG_MORI_HASHIGO, pos.x, pos.y,
                                pos.z, this->dyna.actor.world.rot.x, this->dyna.actor.world.rot.y,
                                this->dyna.actor.world.rot.z, 0);
    if (ladder != NULL) {
        return true;
    } else {
        // "Ladder failure"
        osSyncPrintf("Error : 梯子の発生失敗(%s %d)(arg_data 0x%04x)\n", "../z_bg_mori_hashigo.c", 220,
                     this->dyna.actor.params);
        return false;
    }
}

s32 BgMoriHashigo_InitClasp(BgMoriHashigo* this, GlobalContext* globalCtx) {
    Actor_ProcessInitChain(&this->dyna.actor, sInitChainClasp);
    this->dyna.actor.flags |= ACTOR_FLAG_0;
    Actor_SetFocus(&this->dyna.actor, 55.0f);
    BgMoriHashigo_InitCollider(this, globalCtx);
    if ((this->dyna.actor.params == HASHIGO_CLASP) && !BgMoriHashigo_SpawnLadder(this, globalCtx)) {
        return false;
    } else {
        return true;
    }
}

s32 BgMoriHashigo_InitLadder(BgMoriHashigo* this, GlobalContext* globalCtx) {
    BgMoriHashigo_InitDynapoly(this, globalCtx, &gMoriHashigoCol, DPM_UNK);
    Actor_ProcessInitChain(&this->dyna.actor, sInitChainLadder);
    return true;
}

void BgMoriHashigo_Init(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    BgMoriHashigo* this = (BgMoriHashigo*)thisx;

    if (this->dyna.actor.params == HASHIGO_CLASP) {
        if (!BgMoriHashigo_InitClasp(this, globalCtx)) {
            Actor_Kill(&this->dyna.actor);
            return;
        }
    } else if (this->dyna.actor.params == HASHIGO_LADDER) {
        if (!BgMoriHashigo_InitLadder(this, globalCtx)) {
            Actor_Kill(&this->dyna.actor);
            return;
        }
    }
    this->moriTexObjIndex = Object_GetIndex(&globalCtx->objectCtx, OBJECT_MORI_TEX);
    if (this->moriTexObjIndex < 0) {
        // "Bank danger!"
        osSyncPrintf("Error : バンク危険！(arg_data 0x%04x)(%s %d)\n", this->dyna.actor.params,
                     "../z_bg_mori_hashigo.c", 312);
        Actor_Kill(&this->dyna.actor);
    } else {
        BgMoriHashigo_SetupWaitForMoriTex(this);
        // "(Forest Temple Ladder and its clasp)"
        osSyncPrintf("(森の神殿 梯子とその留め金)(arg_data 0x%04x)\n", this->dyna.actor.params);
    }
}

void BgMoriHashigo_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    BgMoriHashigo* this = (BgMoriHashigo*)thisx;

    if (this->dyna.actor.params == HASHIGO_LADDER) {
        DynaPoly_DeleteBgActor(globalCtx, &globalCtx->colCtx.dyna, this->dyna.bgId);
    }
    if (this->dyna.actor.params == HASHIGO_CLASP) {
        Collider_DestroyJntSph(globalCtx, &this->collider);
    }
}

void BgMoriHashigo_SetupWaitForMoriTex(BgMoriHashigo* this) {
    this->actionFunc = BgMoriHashigo_WaitForMoriTex;
}

void BgMoriHashigo_WaitForMoriTex(BgMoriHashigo* this, GlobalContext* globalCtx) {
    if (Object_IsLoaded(&globalCtx->objectCtx, this->moriTexObjIndex)) {
        if (this->dyna.actor.params == HASHIGO_CLASP) {
            BgMoriHashigo_SetupClasp(this);
        } else if (this->dyna.actor.params == HASHIGO_LADDER) {
            BgMoriHashigo_SetupLadderWait(this);
        }
        this->dyna.actor.draw = BgMoriHashigo_Draw;
    }
}

void BgMoriHashigo_SetupClasp(BgMoriHashigo* this) {
    this->actionFunc = BgMoriHashigo_Clasp;
}

void BgMoriHashigo_Clasp(BgMoriHashigo* this, GlobalContext* globalCtx) {
    if (this->hitTimer <= 0) {
        if (this->collider.base.acFlags & AC_HIT) {
            this->collider.base.acFlags &= ~AC_HIT;
            this->hitTimer = 10;
        } else {
            CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
        }
    }
}

void BgMoriHashigo_SetupLadderWait(BgMoriHashigo* this) {
    this->actionFunc = BgMoriHashigo_LadderWait;
}

void BgMoriHashigo_LadderWait(BgMoriHashigo* this, GlobalContext* globalCtx) {
    BgMoriHashigo* clasp = (BgMoriHashigo*)this->dyna.actor.parent;

    if (clasp->hitTimer > 0) {
        BgMoriHashigo_SetupLadderFall(this);
    }
}

void BgMoriHashigo_SetupLadderFall(BgMoriHashigo* this) {
    this->bounceCounter = 0;
    this->actionFunc = BgMoriHashigo_LadderFall;
    this->dyna.actor.gravity = -1.0f;
    this->dyna.actor.minVelocityY = -10.0f;
    this->dyna.actor.velocity.y = 2.0f;
}

void BgMoriHashigo_LadderFall(BgMoriHashigo* this, GlobalContext* globalCtx) {
    static f32 bounceSpeed[3] = { 4.0f, 2.7f, 1.7f };
    Actor* thisx = &this->dyna.actor;

    Actor_MoveForward(thisx);
    if ((thisx->bgCheckFlags & 1) && (thisx->velocity.y < 0.0f)) {
        if (this->bounceCounter >= ARRAY_COUNT(bounceSpeed)) {
            BgMoriHashigo_SetupLadderRest(this);
        } else {
            Actor_UpdateBgCheckInfo(globalCtx, thisx, 0.0f, 0.0f, 0.0f, 0x1C);
            thisx->velocity.y = bounceSpeed[this->bounceCounter];
            this->bounceCounter++;
        }
    } else {
        Actor_UpdateBgCheckInfo(globalCtx, thisx, 0.0f, 0.0f, 0.0f, 0x1C);
    }
}

void BgMoriHashigo_SetupLadderRest(BgMoriHashigo* this) {
    this->actionFunc = NULL;
    this->dyna.actor.gravity = 0.0f;
    this->dyna.actor.velocity.y = 0.0f;
    this->dyna.actor.world.pos.y = this->dyna.actor.floorHeight;
}

void BgMoriHashigo_Update(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    BgMoriHashigo* this = (BgMoriHashigo*)thisx;

    if (this->hitTimer > 0) {
        this->hitTimer--;
    }
    if (this->actionFunc != NULL) {
        this->actionFunc(this, globalCtx);
    }
}

void BgMoriHashigo_Draw(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    BgMoriHashigo* this = (BgMoriHashigo*)thisx;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_bg_mori_hashigo.c", 516);
    func_80093D18(globalCtx->state.gfxCtx);
    if (1) {}
    gSPSegment(POLY_OPA_DISP++, 0x08, globalCtx->objectCtx.status[this->moriTexObjIndex].segment);

    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_bg_mori_hashigo.c", 521),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    switch (this->dyna.actor.params) {
        case HASHIGO_CLASP:
            gSPDisplayList(POLY_OPA_DISP++, gMoriHashigoClaspDL);
            break;
        case HASHIGO_LADDER:
            gSPDisplayList(POLY_OPA_DISP++, gMoriHashigoLadderDL);
            break;
    }
    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_bg_mori_hashigo.c", 531);
}
