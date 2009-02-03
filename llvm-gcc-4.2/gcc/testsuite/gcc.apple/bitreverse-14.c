/* APPLE LOCAL file 4388773 */
/* { dg-do compile { target powerpc*-*-darwin* } } */
/* { dg-options "-std=gnu99" } */
#pragma reverse_bitfields on
#pragma ms_struct on
#pragma pack(push, 1)

typedef struct _kyc
    {
    void *pkeyres;
    int ipfnctb;
    unsigned short icetab:11,
	    fPrompt:1,
	    coper:4;
    unsigned short ioperArg:14,
	fArg:1,
	fCetab:1;
    int tcidEquiv;
    } KYC;
#pragma pack(pop)

KYC rgkycMain[] =
	{
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,0,0x6,1,0,0,0,1,0},
    {0,0,0x8,1,0,0,0,1,0},
    {0,0,0x9,1,0,0,0,1,0},
    {0,0,0xa,1,0,0,0,1,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0},
    {0,0,0xfe,1,0,0,0,1,0},
    {0,0,0x5,1,0,0,0,1,0},
    };
