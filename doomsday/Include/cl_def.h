#ifndef __DOOMSDAY_CLIENT_H__
#define __DOOMSDAY_CLIENT_H__

#define	SHORTP(x)		(*(short*) (x))
#define	USHORTP(x)		(*(unsigned short*) (x))

extern id_t clientID;
extern int server_time;

// Flags for clmobjs.
#define CLMF_HIDDEN			0x01	// Not officially created yet
#define CLMF_UNPREDICTABLE	0x02	// Temporarily hidden (until next delta)
#define CLMF_SOUND			0x04	// Sound is queued for playing on unhide.
#define CLMF_NULLED			0x08	// Once nulled, it can't be updated.

typedef struct clmobj_s
{
	struct clmobj_s *next, *prev;
	byte flags;	
	uint time;						// Time of last update.
	int sound;						// Queued sound ID.
	float volume;					// Volume for queued sound.
	mobj_t mo;
} clmobj_t;

typedef struct playerstate_s
{
	clmobj_t	*cmo;
	thid_t		mobjid;
	int			forwardmove;
	int			sidemove;
	int			angle;
	angle_t		turndelta;
	int			friction;
} playerstate_t;

// 
// cl_main.c
//
extern boolean handshake_received;
extern int game_ready;
extern boolean net_loggedin;
extern boolean clientPaused;

void Cl_InitID(void);
void Cl_CleanUp();
void Cl_GetPackets(void);
void Cl_Ticker(void);
int Cl_GameReady();
void Cl_SendHello(void);

//
// cl_player.c
//
extern int psp_move_speed;
extern int cplr_thrust_mul;
extern playerstate_t playerstate[MAXPLAYERS];

void Cl_InitPlayers(void);
void Cl_LocalCommand(void);
void Cl_MovePlayer(ddplayer_t *pl);
void Cl_MoveLocalPlayer(int dx, int dy, int dz, boolean onground);
int Cl_ReadPlayerDelta(void);
void Cl_ReadPlayerDelta2(void);
void Cl_UpdatePlayerPos(ddplayer_t *pl);
void Cl_MovePsprites(void);
void Cl_CoordsReceived(void);

#endif