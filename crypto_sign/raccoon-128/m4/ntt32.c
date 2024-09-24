//  ntt32.c
//  Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

//  === 32-bit Number Theoretic Transform

#include <stddef.h>
#include <stdbool.h>

#include "polyr.h"
#include "mont32.h"
#include "mont64.h"

//  === Roots of unity constants

/*
    (g is the smallest full generator for both q1 and q2)

    n   = 512
    q1  = 2^24-2^18+1
    q2  = 2^25-2^18+1
    q   = q1*q2
    r   = 2^64 % q
    r1  = 2^32 % q1
    r2  = 2^32 % q2
    g   = Mod(15, q)
    h   = g^(znorder(g)/(2*n))
    h1  = Mod(lift(h), q1)
    h2  = Mod(lift(h), q2)

    bitrev(n,i) = \
      b = binary(n + (i%n));\
      sum(i = 2, length(b), 2^(i-2) * b[i])

    Generating the Montgomery ("r1") and ("r2") scaled constants
    racc_w32_1[511] and racc_w32_2[511]:

    w1 = vector(511,i,lift(r1*h1^bitrev(n,i)))
    w2 = vector(511,i,lift(r2*h2^bitrev(n,i)))

    print them out

    for(i=1,511,printf("\t{%d, %d},", w1[i], w2[i]);if(i%3==0,printf("\n")))
*/

static const int32_t racc_w_32[511][2] = {
    {6459829, 18304632},  {6724791, 7543113},   {2854072, 7433152},
    {2659044, 10516699},  {14713997, 30041014}, {14113155, 12016999},
    {13853133, 30119652}, {2214999, 25003819},  {713782, 7062373},
    {12189159, 4939132},  {13885804, 28572885}, {12118639, 27402401},
    {16472663, 27260573}, {13975435, 25227641}, {15691979, 1699052},
    {9507138, 23584536},  {2513236, 16900132},  {4580230, 2829345},
    {13325401, 21662135}, {13264360, 11299231}, {16421508, 9488343},
    {7465801, 20663113},  {14835457, 20614042}, {9534672, 17051604},
    {3986098, 27372078},  {164372, 11875826},   {12020825, 15152935},
    {2053709, 27749958},  {759182, 19414638},   {335953, 14639878},
    {1160782, 8428013},   {7725862, 17539385},  {5235549, 31212134},
    {6044843, 16328186},  {2999387, 24184363},  {13303838, 28498243},
    {5379527, 24092323},  {11057583, 22582647}, {16442646, 25731287},
    {10414489, 11403006}, {342049, 7289164},    {2248971, 1926796},
    {1649313, 19241740},  {5959367, 8834508},   {6362504, 5015735},
    {10472383, 5234908},  {2359291, 32245547},  {14068601, 10130955},
    {10573718, 22308470}, {8592974, 5450924},   {8185132, 22839876},
    {14785052, 403006},   {1976795, 9839231},   {9932237, 24442133},
    {15129302, 4947299},  {14607709, 25177546}, {11819643, 22484191},
    {2017545, 18488793},  {15516096, 116842},   {13739185, 6357874},
    {16416870, 32862008}, {3643612, 14154844},  {10630888, 23872154},
    {13065073, 18735062}, {13207150, 18328159}, {6641700, 11788671},
    {3224575, 19484597},  {6980454, 3230630},   {8058453, 30533374},
    {236196, 22977545},   {13178135, 12768015}, {16470624, 846457},
    {13727118, 27802498}, {6825156, 23026243},  {3119691, 28339012},
    {15892217, 5642243},  {3127331, 19197405},  {15835742, 28664087},
    {10111116, 20436312}, {9705449, 16991644},  {12955837, 9142167},
    {1143885, 25162084},  {10229685, 16882607}, {1554252, 10219044},
    {8155384, 1033849},   {6824996, 15369493},  {9448688, 21427015},
    {11607389, 2798453},  {15856809, 31407969}, {11728448, 16558299},
    {11532439, 11100836}, {14049335, 13514388}, {5332490, 11750397},
    {9437148, 15183463},  {9940094, 31390971},  {61359, 3690205},
    {1720349, 25550194},  {1945542, 25611170},  {1724483, 30470884},
    {12882462, 334634},   {3111340, 2837321},   {15935803, 28496418},
    {4235249, 4978508},   {3240775, 32736283},  {763969, 20888702},
    {14333122, 16278794}, {12411974, 16212395}, {3368434, 4695507},
    {3738409, 1750758},   {8826952, 6760130},   {1544575, 30285659},
    {10531868, 5122156},  {9464026, 27441713},  {10348034, 14062271},
    {7799654, 14420085},  {1169657, 26689043},  {1096084, 26365212},
    {4571137, 16771475},  {3795111, 23485566},  {15500529, 29454327},
    {14773253, 27161931}, {15751114, 32851742}, {8006173, 2144336},
    {12738660, 6504132},  {75912, 12154117},    {11924466, 20944052},
    {15961082, 17934007}, {15575459, 14384724}, {16312641, 18472847},
    {1231570, 19080737},  {12487358, 14395273}, {2037224, 12139621},
    {6175327, 25192616},  {15654902, 27813450}, {8304598, 27753487},
    {10940413, 8414681},  {1221020, 21677243},  {2287719, 12947324},
    {6064884, 5391700},   {15335423, 20600271}, {6394777, 840503},
    {8087158, 24210537},  {14129134, 27217272}, {11326961, 8542770},
    {906403, 18915968},   {917427, 23752522},   {163624, 4757196},
    {1544720, 8326415},   {10726100, 4066115},  {2713142, 12843560},
    {1323086, 226722},    {5818536, 22227227},  {5436573, 19269447},
    {8972798, 84067},     {14147091, 4919274},  {3986583, 3607977},
    {6891182, 3133026},   {1040958, 7065810},   {14487515, 25678085},
    {15318946, 19859590}, {2411182, 5295285},   {8821128, 19895385},
    {7315073, 3027303},   {15885217, 15457740}, {3393477, 24518905},
    {11540938, 24574131}, {2099471, 12296673},  {14829730, 31605980},
    {8195897, 4906188},   {12939571, 14099108}, {10891518, 7650520},
    {12821598, 18914015}, {6067170, 21716563},  {13680548, 11483061},
    {3844075, 7495071},   {13464713, 33011439}, {5750986, 602173},
    {9491296, 11960669},  {3861100, 12055028},  {3820108, 22726698},
    {13338669, 14653790}, {5777431, 2223715},   {4144672, 32513905},
    {7259309, 926362},    {13287024, 25525289}, {7260395, 8812935},
    {8932940, 16772554},  {7864418, 1103720},   {12028253, 31363210},
    {7800124, 18705188},  {15444796, 24506803}, {15097370, 26213179},
    {3399190, 31588038},  {10298911, 22623386}, {13805558, 30732081},
    {11910139, 26502754}, {12603531, 31153860}, {15812918, 29811910},
    {7090784, 20157716},  {525722, 16133110},   {7011384, 700933},
    {9106630, 33243631},  {5906664, 27261400},  {6772368, 17370981},
    {13204966, 23562435}, {11123142, 16460624}, {4791249, 3305769},
    {15311175, 13251893}, {8505792, 12133625},  {4308086, 31999666},
    {8837295, 22483618},  {7616319, 17405866},  {4478976, 4010730},
    {5754531, 6426366},   {7846505, 1899664},   {5066699, 10284450},
    {6479637, 26408651},  {380454, 27001711},   {8910719, 10483140},
    {10114174, 22233305}, {7914630, 24003515},  {3480539, 12904817},
    {16486606, 24834805}, {5900516, 5940897},   {14021732, 30118142},
    {2553609, 2079905},   {9592250, 24498288},  {3543258, 15463836},
    {5698163, 20229106},  {7674692, 8182961},   {13456804, 21031339},
    {10517817, 22288090}, {12116905, 23150502}, {10097918, 8388731},
    {246849, 24897932},   {16113840, 6834477},  {4718606, 14126630},
    {5053420, 14912019},  {10386621, 18132346}, {15620573, 1619174},
    {5698103, 13452201},  {10338589, 9577605},  {15496172, 17473021},
    {12049205, 17597750}, {14705438, 18599273}, {233571, 13239791},
    {891635, 8886177},    {11134953, 9843456},  {8168963, 25148376},
    {1511098, 2772912},   {11428769, 8994793},  {1293750, 23381116},
    {15691160, 7545971},  {5766899, 22141026},  {9112705, 11428204},
    {3461385, 8714786},   {11769602, 2462532},  {2749974, 10106905},
    {10926459, 12592243}, {13696503, 12947949}, {10103004, 4640587},
    {3257614, 2443992},   {8801227, 30611882},  {1813360, 5553419},
    {3532076, 14987842},  {10123210, 1945751},  {10100239, 28955259},
    {13876606, 5264857},  {10819237, 27308461}, {16451816, 19088878},
    {15348184, 15105305}, {6660339, 15031281},  {10249677, 10851174},
    {13856284, 15952833}, {7713214, 21116302},  {3877381, 3599602},
    {10089980, 19856398}, {4574306, 11769977},  {4472065, 28740307},
    {5974238, 29789489},  {14625582, 9442606},  {10376258, 14577539},
    {7783518, 12073694},  {16514990, 32931525}, {6689401, 24037460},
    {619116, 7487648},    {14756774, 12565774}, {14866413, 6972290},
    {2946088, 26739211},  {6936450, 21634722},  {2634760, 14347844},
    {4749457, 25018769},  {10321662, 10402815}, {1126179, 20293655},
    {5212260, 8597115},   {11240507, 10943150}, {13847698, 21825350},
    {9668828, 7727370},   {11417953, 4074437},  {13323795, 5162670},
    {16214326, 28717833}, {7645780, 20491807},  {12612003, 6650661},
    {14582530, 27776321}, {9573046, 18147926},  {8963574, 25925207},
    {1644350, 18163805},  {5923010, 7924792},   {8419587, 27774746},
    {4293933, 18925322},  {15731348, 18461954}, {11878204, 9517491},
    {4687251, 2464441},   {11271718, 2926065},  {13656652, 18087002},
    {6806179, 19052850},  {13386925, 25057507}, {5849341, 27931455},
    {3345215, 23634251},  {12235184, 8544621},  {14613155, 17597022},
    {10785538, 27196855}, {2980732, 3575549},   {5806441, 9863708},
    {5448169, 7947778},   {1693548, 10402959},  {4255934, 2667162},
    {5684718, 13349263},  {13742528, 30416091}, {2518115, 9546618},
    {13329370, 21207243}, {15832424, 27318421}, {11096053, 6469516},
    {3601885, 9033833},   {13505560, 19823865}, {2599878, 28680807},
    {4383772, 13126776},  {385990, 2988456},    {11229401, 6868088},
    {1833169, 9259841},   {6596262, 9809280},   {4034814, 2749909},
    {8506689, 12615641},  {14062146, 15168416}, {6312887, 8244996},
    {7794008, 2150793},   {974198, 5677958},    {3366379, 18803214},
    {15353500, 6151764},  {12615014, 2252928},  {13525309, 11841819},
    {8459615, 28117011},  {13920877, 7155340},  {11638972, 6768109},
    {7969751, 12333627},  {5783292, 22216219},  {12703132, 3947183},
    {11206276, 12730841}, {11699899, 6622706},  {10436920, 11300381},
    {10960101, 17428136}, {9433646, 3917613},   {12423102, 16516499},
    {3536472, 7507568},   {2247359, 2742231},   {13722220, 11894516},
    {7789860, 11601672},  {6826300, 4312728},   {13192857, 18686795},
    {8974319, 12024439},  {2391871, 6796134},   {7525685, 31188053},
    {7292667, 8630780},   {13451270, 8619954},  {4171764, 10214090},
    {10418725, 13895612}, {10731765, 6803985},  {299218, 18087074},
    {1448609, 27563045},  {1693871, 25286329},  {12845647, 2867214},
    {13186654, 25258840}, {6607446, 28552029},  {988035, 15835705},
    {9785136, 31319443},  {8875784, 15585909},  {15410290, 7545557},
    {10212134, 2714668},  {1642067, 7186505},   {4412106, 27922344},
    {258145, 24622623},   {14603653, 30229135}, {4631726, 2148758},
    {14364267, 10667779}, {7193418, 7479943},   {2665117, 14314799},
    {5390369, 3962194},   {13583034, 9561457},  {14563469, 8061572},
    {13002936, 6033676},  {767428, 28775647},   {6917856, 32002897},
    {538876, 17375508},   {14156792, 19351181}, {12840464, 22197221},
    {10165176, 260611},   {15422112, 23028189}, {13378781, 9479765},
    {16416947, 20263442}, {3803210, 18141718},  {306665, 3068838},
    {7447608, 21390640},  {12257186, 15028368}, {16027140, 17169085},
    {9430592, 4812752},   {15278760, 4871381},  {3061430, 26524309},
    {16081830, 2074088},  {10607060, 5441160},  {11998945, 21946142},
    {10670940, 13761919}, {12996671, 19809115}, {4473794, 2033976},
    {15305124, 25344249}, {8495208, 643663},    {1888429, 15622808},
    {15159248, 28880075}, {1405022, 685396},    {8025368, 10316037},
    {14450720, 17559381}, {3684627, 12880965},  {9980742, 7122242},
    {8590101, 14802322},  {15835063, 22401669}, {6933008, 22113106},
    {5755569, 23780078},  {10345642, 25015122}, {15440114, 28752060},
    {10393475, 31421498}, {1236076, 10290411},  {2907163, 6567132},
    {5185732, 29946207},  {4468910, 19843571},  {4895133, 13856329},
    {7972704, 22798690},  {3097528, 29919843},  {14482035, 30859467},
    {9181809, 12767929},  {3892922, 16437343},  {7149565, 32761422},
    {9230360, 691455},    {2323078, 30187676},  {4055183, 25179754},
    {6088802, 6418749},   {3493862, 19629722},  {529895, 28492891},
    {591151, 32708228},   {12601228, 7875335},  {10340651, 20946247},
    {11178734, 6541367},  {3111704, 20407133},  {15188896, 33142871},
    {9479161, 32762006},  {7090104, 25737367},  {6956551, 29237631},
    {5697674, 29165600},  {1245831, 16775749},  {9821948, 27169550},
    {15599615, 33194114}, {11628557, 25818405}, {15844707, 882609},
    {10865149, 3857404},  {12274098, 28788596}, {10543667, 24207723},
    {10426277, 28574912}, {14864097, 20650342}, {15698846, 14282046},
    {9686719, 27541825},  {5504803, 14605849},  {14532946, 28192158},
    {3984022, 15337026},  {3153814, 12622799},  {6613622, 26521052},
    {13511929, 12970664}, {2615713, 12232455},  {5505714, 19081762},
    {1655170, 9168373},   {12751629, 4245406},  {13592320, 29066697},
    {7113000, 21054226},  {7954588, 20455998},  {7136375, 22327431},
    {15913211, 7672038},  {801992, 31898677},   {2499974, 29963997},
    {9127282, 4591219},   {1120091, 11437979},  {1996999, 25728942},
    {11361625, 743579},   {5064585, 19506115},  {13099982, 13275295},
    {10577914, 9542626},  {10289669, 21263930}, {13982280, 28402557},
    {7622456, 1282478},   {4015737, 16171986},  {4150089, 24134014},
    {11926879, 9271102},  {13099524, 22217378}, {8341728, 19711414},
    {7139762, 14671670}};

//  2x32 CRT: Split into two-prime representation (in-place).
extern void polyr2_split_asm(int64_t* v);
void polyr2_split(int64_t *v)
{
    polyr2_split_asm(v);
    // int64_t x;
    // int32_t *p0, *p1;

    // p0 = (int32_t *)v;
    // p1 = (int32_t *)(v + RACC_N);

    // while (p0 < p1) {
    //     x = *((int64_t *)p0);
    //     p0[0] = mont32_redc1(x);
    //     p0[1] = mont32_redc2(x);
    //     p0 += 2;
    // }
}

//  2x32 CRT: Join two-prime into 64-bit integer representation (in-place).
//  Use scale factors (s1, s2). Normalizes to 0 <= x < q.

extern void polyr2_join_asm(int64_t* v, int32_t s1, int32_t s2);
void polyr2_join(int64_t *v, int32_t s1, int32_t s2)
{
    polyr2_join_asm(v,s1,s2);

    // int64_t x;
    // int32_t x1, x2;

    // int32_t *p0, *p1;

    // p0 = (int32_t *)v;
    // p1 = (int32_t *)(v + RACC_N);

    // while (p0 < p1) {
    //     x1 = mont32_cadd(mont32_mulq1(p0[0], s1), RACC_Q1);
    //     x2 = mont32_cadd(mont32_mulq2(p0[1], s2), RACC_Q2);

    //     x = (((int64_t)RACC_Q2) * ((int64_t)x1)) +
    //         (((int64_t)RACC_Q1) * ((int64_t)x2));

    //     //  we have [0,2q], put to [0,q-1]
    //     x = mont64_csub(x, RACC_Q);

    //     *((int64_t *)p0) = x;
    //     p0 += 2;
    // }
}

//  2x32 CRT: Add polynomials:  r = a + b.
extern void polyr2_add_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr2_add(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr2_add_asm(r,a,b);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;
    // const int32_t *b2 = (const int32_t *)b;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_add(a2[i + 0], b2[i + 0]);
    //     r2[i + 1] = mont32_add(a2[i + 1], b2[i + 1]);
    // }
}

//  2x32 CRT: Subtract polynomials:  r = a - b.
extern void polyr2_sub_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr2_sub(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr2_sub_asm(r,a,b);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;
    // const int32_t *b2 = (const int32_t *)b;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_sub(a2[i + 0], b2[i + 0]);
    //     r2[i + 1] = mont32_sub(a2[i + 1], b2[i + 1]);
    // }
}

//  2x32 CRT: Add polynomials mod q1 and q2: r = a + b  (mod q).
extern void polyr_ntt_addq_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_ntt_addq(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_ntt_addq_asm(r,a,b);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;
    // const int32_t *b2 = (const int32_t *)b;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_csub(mont32_add(a2[i + 0], b2[i + 0]), RACC_Q1);
    //     r2[i + 1] = mont32_csub(mont32_add(a2[i + 1], b2[i + 1]), RACC_Q2);
    // }
}

//  2x32 CRT: Subtract polynomials mod q1 and q2: r = a - b (mod q).
extern void polyr_ntt_subq_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_ntt_subq(int64_t *r, const int64_t *a, const int64_t *b)
{
    polyr_ntt_subq_asm(r,a,b);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;
    // const int32_t *b2 = (const int32_t *)b;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_cadd(mont32_sub(a2[i + 0], b2[i + 0]), RACC_Q1);
    //     r2[i + 1] = mont32_cadd(mont32_sub(a2[i + 1], b2[i + 1]), RACC_Q2);
    // }
}

//  2x32 CRT: Scalar multiplication:    r = a * c,  Montgomery reduction.
extern void polyr_ntt_smul_asm(int64_t *r, const int64_t *a, int32_t c1, int32_t c2);
void polyr_ntt_smul(int64_t *r, const int64_t *a, int32_t c1, int32_t c2)
{
    polyr_ntt_smul_asm(r,a,c1,c2);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_cadd(mont32_mulq1(a2[i + 0], c1), RACC_Q1);
    //     r2[i + 1] = mont32_cadd(mont32_mulq2(a2[i + 1], c2), RACC_Q2);
    // }
}

//  2x32 CRT: Coefficient multiply:  r = a * b,  Montgomery reduction.
extern void polyr_ntt_cmul_asm(int64_t *r, const int64_t *a, const int64_t *b);
void polyr_ntt_cmul(int64_t *r, const int64_t *a, const int64_t *b)
{
    // polyr_ntt_cmul_asm(r,a,b);
    size_t i;
    int32_t *r2 = (int32_t *)r;
    const int32_t *a2 = (const int32_t *)a;
    const int32_t *b2 = (const int32_t *)b;

    for (i = 0; i < (2 * RACC_N); i += 2) {
        r2[i + 0] = mont32_mulq1(a2[i + 0], b2[i + 0]);
        r2[i + 1] = mont32_mulq2(a2[i + 1], b2[i + 1]);
    }
}

//  2x32 CRT: Multiply and add:  r = a * b + c, Montgomery reduction.
extern void polyr_ntt_mula_asm(int64_t *r, const int64_t *a, const int64_t *b, const int64_t *c);
void polyr_ntt_mula(int64_t *r, const int64_t *a, const int64_t *b,
                    const int64_t *c)
{
    polyr_ntt_mula_asm(r,a,b,c);
    // size_t i;
    // int32_t *r2 = (int32_t *)r;
    // const int32_t *a2 = (const int32_t *)a;
    // const int32_t *b2 = (const int32_t *)b;
    // const int32_t *c2 = (const int32_t *)c;

    // for (i = 0; i < (2 * RACC_N); i += 2) {
    //     r2[i + 0] = mont32_csub(mont32_mulq1(a2[i + 0], b2[i + 0]) + c2[i + 0],
    //                             RACC_Q1);
    //     r2[i + 1] = mont32_csub(mont32_mulq2(a2[i + 1], b2[i + 1]) + c2[i + 1],
    //                             RACC_Q2);
    // }
}

//  2x32 CRT: Forward NTT (x^n+1). Input is 64-bit, output is 2x32 CRT.
extern void raccoon_ntt(int64_t p[512]);
void polyr_fntt(int64_t *v)
{
    raccoon_ntt(v);
    // size_t i, j, k;
    // int64_t x;
    // int32_t x1, x2, y1, y2, z1, z2;
    // int32_t *p0, *p1, *p2;

    // const int32_t *w = racc_w_32[0];

    // //  split

    // p0 = (int32_t *)v;
    // p1 = (int32_t *)(v + RACC_N);

    // while (p0 < p1) {
    //     x = *((int64_t *)p0);
    //     p0[0] = mont32_redc1(x);
    //     p0[1] = mont32_redc2(x);
    //     p0 += 2;
    // }

    // //  NTT butterflies

    // for (k = 1, j = RACC_N; j > 1; k <<= 1, j >>= 1) {

    //     p0 = (int32_t *)v;
    //     for (i = 0; i < k; i++) {
    //         z1 = w[0];
    //         z2 = w[1];
    //         w += 2;

    //         p1 = p0 + j;
    //         p2 = p1 + j;

    //         while (p1 < p2) {
    //             x1 = p0[0];
    //             x2 = p0[1];
    //             y1 = p1[0];
    //             y2 = p1[1];
    //             y1 = mont32_mulq1(y1, z1);
    //             y2 = mont32_mulq2(y2, z2);
    //             p0[0] = mont32_add(x1, y1);
    //             p0[1] = mont32_add(x2, y2);
    //             p1[0] = mont32_sub(x1, y1);
    //             p1[1] = mont32_sub(x2, y2);
    //             p0 += 2;
    //             p1 += 2;
    //         }
    //         p0 = p2;
    //     }
    // }
}

//  2x32 CRT: Inverse NTT (x^n+1).
extern void raccoon_invntt(int64_t *v);
void polyr_intt(int64_t *v)
{
    raccoon_invntt(v);
    // size_t i, j, k;
    // int64_t x;
    // int32_t x1, x2, y1, y2, z1, z2;
    // int32_t *p0, *p1, *p2;

    // const int32_t *w = racc_w_32[RACC_N - 2];

    // //  inverse butterflies

    // for (j = 2, k = RACC_N >> 1; k > 0; j <<= 1, k >>= 1) {

    //     p0 = (int32_t *)v;
    //     for (i = 0; i < k; i++) {
    //         z1 = w[0];
    //         z2 = w[1];
    //         w -= 2;

    //         p1 = p0 + j;
    //         p2 = p1 + j;

    //         while (p1 < p2) {
    //             x1 = mont32_cadd(p0[0], RACC_Q1);
    //             x2 = mont32_cadd(p0[1], RACC_Q2);
    //             y1 = p1[0];
    //             y2 = p1[1];
    //             p0[0] = mont32_csub(mont32_add(x1, y1), RACC_Q1);
    //             p0[1] = mont32_csub(mont32_add(x2, y2), RACC_Q2);
    //             p1[0] = mont32_mulq1(mont32_sub(y1, x1), z1);
    //             p1[1] = mont32_mulq2(mont32_sub(y2, x2), z2);
    //             p0 += 2;
    //             p1 += 2;
    //         }
    //         p0 = p2;
    //     }
    // }

    // //  join & normalize
    // p0 = (int32_t *)v;
    // p1 = (int32_t *)(v + RACC_N);

    // while (p0 < p1) {
    //     x1 = mont32_cadd(mont32_mulq1(p0[0], MONT_C4Q1), RACC_Q1);
    //     x2 = mont32_cadd(mont32_mulq2(p0[1], MONT_C4Q2), RACC_Q2);

    //     x = (((int64_t)RACC_Q2) * ((int64_t)x1)) +
    //         (((int64_t)RACC_Q1) * ((int64_t)x2));

    //     // we have [0,~2q], put to [0,q-1]
    //     x = mont64_csub(x, RACC_Q);
    //     *((int64_t *)p0) = x;
    //     p0 += 2;
    // }
}

