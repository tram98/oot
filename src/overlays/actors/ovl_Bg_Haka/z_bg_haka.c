/*
 * File: z_bg_haka.c
 * Overlay: ovl_Bg_Haka
 * Description: Gravestone
 */

#include "z_bg_haka.h"
#include "objects/object_haka/object_haka.h"

#define FLAGS 0

void BgHaka_Init(Actor* thisx, GlobalContext* globalCtx);
void BgHaka_Destroy(Actor* thisx, GlobalContext* globalCtx);
void BgHaka_Update(Actor* thisx, GlobalContext* globalCtx);
void BgHaka_Draw(Actor* thisx, GlobalContext* globalCtx);

void func_8087B758(BgHaka* this, Player* player);
void func_8087B7E8(BgHaka* this, GlobalContext* globalCtx);
void func_8087B938(BgHaka* this, GlobalContext* globalCtx);
void func_8087BAAC(BgHaka* this, GlobalContext* globalCtx);
void func_8087BAE4(BgHaka* this, GlobalContext* globalCtx);

const ActorInit Bg_Haka_InitVars = {
    ACTOR_BG_HAKA,
    ACTORCAT_BG,
    FLAGS,
    OBJECT_HAKA,
    sizeof(BgHaka),
    (ActorFunc)BgHaka_Init,
    (ActorFunc)BgHaka_Destroy,
    (ActorFunc)BgHaka_Update,
    (ActorFunc)BgHaka_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(minVelocityY, 0, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

void BgHaka_Init(Actor* thisx, GlobalContext* globalCtx) {
    BgHaka* this = (BgHaka*)thisx;
    s32 pad;
    CollisionHeader* colHeader = NULL;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    DynaPolyActor_Init(&this->dyna, DPM_UNK);
    CollisionHeader_GetVirtual(&gGravestoneCol, &colHeader);
    this->dyna.bgId = DynaPoly_SetBgActor(globalCtx, &globalCtx->colCtx.dyna, &this->dyna.actor, colHeader);
    this->actionFunc = func_8087B7E8;
}

void BgHaka_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    BgHaka* this = (BgHaka*)thisx;

    DynaPoly_DeleteBgActor(globalCtx, &globalCtx->colCtx.dyna, this->dyna.bgId);
}

void func_8087B758(BgHaka* this, Player* player) {
    Vec3f sp1C;

    func_8002DBD0(&this->dyna.actor, &sp1C, &player->actor.world.pos);
    if (fabsf(sp1C.x) < 34.6f && sp1C.z > -112.8f && sp1C.z < -36.0f) {
        player->stateFlags2 |= 0x200;
    }
}

void func_8087B7E8(BgHaka* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);

    if (this->dyna.unk_150 != 0.0f) {
        if (globalCtx->sceneNum == SCENE_SPOT02 && !LINK_IS_ADULT && IS_DAY) {
            this->dyna.unk_150 = 0.0f;
            player->stateFlags2 &= ~0x10;
            if (!Gameplay_InCsMode(globalCtx)) {
                Message_StartTextbox(globalCtx, 0x5073, NULL);
                this->dyna.actor.params = 100;
                this->actionFunc = func_8087BAE4;
            }
        } else if (0.0f < this->dyna.unk_150 ||
                   (globalCtx->sceneNum == SCENE_SPOT06 && !LINK_IS_ADULT && !Flags_GetSwitch(globalCtx, 0x23))) {
            this->dyna.unk_150 = 0.0f;
            player->stateFlags2 &= ~0x10;
        } else {
            this->dyna.actor.world.rot.y = this->dyna.actor.shape.rot.y + 0x8000;
            this->actionFunc = func_8087B938;
        }
    }
    func_8087B758(this, player);
}

void func_8087B938(BgHaka* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);
    s32 sp38;

    this->dyna.actor.speedXZ += 0.05f;
    this->dyna.actor.speedXZ = CLAMP_MAX(this->dyna.actor.speedXZ, 1.5f);
    sp38 = Math_StepToF(&this->dyna.actor.minVelocityY, 60.0f, this->dyna.actor.speedXZ);
    this->dyna.actor.world.pos.x =
        Math_SinS(this->dyna.actor.world.rot.y) * this->dyna.actor.minVelocityY + this->dyna.actor.home.pos.x;
    this->dyna.actor.world.pos.z =
        Math_CosS(this->dyna.actor.world.rot.y) * this->dyna.actor.minVelocityY + this->dyna.actor.home.pos.z;
    if (sp38 != 0) {
        this->dyna.unk_150 = 0.0f;
        player->stateFlags2 &= ~0x10;
        if (this->dyna.actor.params == 1) {
            func_80078884(NA_SE_SY_CORRECT_CHIME);
        } else if (!IS_DAY && globalCtx->sceneNum == SCENE_SPOT02) {
            Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_POH, this->dyna.actor.home.pos.x,
                        this->dyna.actor.home.pos.y, this->dyna.actor.home.pos.z, 0, this->dyna.actor.shape.rot.y, 0,
                        1);
        }
        this->actionFunc = func_8087BAAC;
    }
    func_8002F974(&this->dyna.actor, NA_SE_EV_ROCK_SLIDE - SFX_FLAG);
}

void func_8087BAAC(BgHaka* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);

    if (this->dyna.unk_150 != 0.0f) {
        this->dyna.unk_150 = 0.0f;
        player->stateFlags2 &= ~0x10;
    }
}

void func_8087BAE4(BgHaka* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);
    s32 pad;

    if (this->dyna.actor.params != 0) {
        this->dyna.actor.params -= 1;
    }
    if (this->dyna.unk_150 != 0.0f) {
        this->dyna.unk_150 = 0.0f;
        player->stateFlags2 &= ~0x10;
    }
    if (this->dyna.actor.params == 0) {
        this->actionFunc = func_8087B7E8;
    }
    func_8087B758(this, player);
}

void BgHaka_Update(Actor* thisx, GlobalContext* globalCtx) {
    BgHaka* this = (BgHaka*)thisx;

    this->actionFunc(this, globalCtx);
}

void BgHaka_Draw(Actor* thisx, GlobalContext* globalCtx) {
    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_bg_haka.c", 401);

    func_80093D18(globalCtx->state.gfxCtx);
    func_80093D84(globalCtx->state.gfxCtx);

    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_bg_haka.c", 406),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, gGravestoneStoneDL);
    Matrix_Translate(0.0f, 0.0f, thisx->minVelocityY * 10.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_bg_haka.c", 416),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, gGravestoneEarthDL);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_bg_haka.c", 421);
}
