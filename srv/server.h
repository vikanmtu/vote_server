//#define SRV_VLOG 1

#define SRV_ALLOW_REGS 0x01
#define SRV_ALLOW_JOIN 0x02
#define SRV_ALLOW_VOTE 0x04
#define SRV_ALLOW_OPEN 0x08
#define SRV_ALLOW_WEBS 0x10


short srv_init(int argc, char* argv[]);
void srv_process(void);

short tor_run(void);



