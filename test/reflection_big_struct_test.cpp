#if defined(__clang__) && __clang_major__ >= 15

#define FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024 1

#include "fixed_containers/reflection.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>

namespace fixed_containers
{
namespace
{
struct FieldCount100
{
    // clang-format off
    bool a1;bool a2;bool a3;bool a4;bool a5;bool a6;bool a7;bool a8;bool a9;bool a10;bool a11;bool a12;bool a13;bool a14;bool a15;bool a16;bool a17;bool a18;bool a19;bool a20;bool a21;bool a22;bool a23;bool a24;bool a25;bool a26;bool a27;bool a28;bool a29;bool a30;bool a31;bool a32;bool a33;bool a34;bool a35;bool a36;bool a37;bool a38;bool a39;bool a40;bool a41;bool a42;bool a43;bool a44;bool a45;bool a46;bool a47;bool a48;bool a49;bool a50;bool a51;bool a52;bool a53;bool a54;bool a55;bool a56;bool a57;bool a58;bool a59;bool a60;bool a61;bool a62;bool a63;bool a64;bool a65;bool a66;bool a67;bool a68;bool a69;bool a70;bool a71;bool a72;bool a73;bool a74;bool a75;bool a76;bool a77;bool a78;bool a79;bool a80;bool a81;bool a82;bool a83;bool a84;bool a85;bool a86;bool a87;bool a88;bool a89;bool a90;bool a91;bool a92;bool a93;bool a94;bool a95;bool a96;bool a97;bool a98;bool a99;bool a100;  // clang-format on
    // clang-format on
};

struct FieldCount1000
{
    // clang-format off
    bool a1;bool a2;bool a3;bool a4;bool a5;bool a6;bool a7;bool a8;bool a9;bool a10;bool a11;bool a12;bool a13;bool a14;bool a15;bool a16;bool a17;bool a18;bool a19;bool a20;bool a21;bool a22;bool a23;bool a24;bool a25;bool a26;bool a27;bool a28;bool a29;bool a30;bool a31;bool a32;bool a33;bool a34;bool a35;bool a36;bool a37;bool a38;bool a39;bool a40;bool a41;bool a42;bool a43;bool a44;bool a45;bool a46;bool a47;bool a48;bool a49;bool a50;bool a51;bool a52;bool a53;bool a54;bool a55;bool a56;bool a57;bool a58;bool a59;bool a60;bool a61;bool a62;bool a63;bool a64;bool a65;bool a66;bool a67;bool a68;bool a69;bool a70;bool a71;bool a72;bool a73;bool a74;bool a75;bool a76;bool a77;bool a78;bool a79;bool a80;bool a81;bool a82;bool a83;bool a84;bool a85;bool a86;bool a87;bool a88;bool a89;bool a90;bool a91;bool a92;bool a93;bool a94;bool a95;bool a96;bool a97;bool a98;bool a99;bool a100;bool a101;bool a102;bool a103;bool a104;bool a105;bool a106;bool a107;bool a108;bool a109;bool a110;bool a111;bool a112;bool a113;bool a114;bool a115;bool a116;bool a117;bool a118;bool a119;bool a120;bool a121;bool a122;bool a123;bool a124;bool a125;bool a126;bool a127;bool a128;bool a129;bool a130;bool a131;bool a132;bool a133;bool a134;bool a135;bool a136;bool a137;bool a138;bool a139;bool a140;bool a141;bool a142;bool a143;bool a144;bool a145;bool a146;bool a147;bool a148;bool a149;bool a150;bool a151;bool a152;bool a153;bool a154;bool a155;bool a156;bool a157;bool a158;bool a159;bool a160;bool a161;bool a162;bool a163;bool a164;bool a165;bool a166;bool a167;bool a168;bool a169;bool a170;bool a171;bool a172;bool a173;bool a174;bool a175;bool a176;bool a177;bool a178;bool a179;bool a180;bool a181;bool a182;bool a183;bool a184;bool a185;bool a186;bool a187;bool a188;bool a189;bool a190;bool a191;bool a192;bool a193;bool a194;bool a195;bool a196;bool a197;bool a198;bool a199;bool a200;bool a201;bool a202;bool a203;bool a204;bool a205;bool a206;bool a207;bool a208;bool a209;bool a210;bool a211;bool a212;bool a213;bool a214;bool a215;bool a216;bool a217;bool a218;bool a219;bool a220;bool a221;bool a222;bool a223;bool a224;bool a225;bool a226;bool a227;bool a228;bool a229;bool a230;bool a231;bool a232;bool a233;bool a234;bool a235;bool a236;bool a237;bool a238;bool a239;bool a240;bool a241;bool a242;bool a243;bool a244;bool a245;bool a246;bool a247;bool a248;bool a249;bool a250;bool a251;bool a252;bool a253;bool a254;bool a255;bool a256;bool a257;bool a258;bool a259;bool a260;bool a261;bool a262;bool a263;bool a264;bool a265;bool a266;bool a267;bool a268;bool a269;bool a270;bool a271;bool a272;bool a273;bool a274;bool a275;bool a276;bool a277;bool a278;bool a279;bool a280;bool a281;bool a282;bool a283;bool a284;bool a285;bool a286;bool a287;bool a288;bool a289;bool a290;bool a291;bool a292;bool a293;bool a294;bool a295;bool a296;bool a297;bool a298;bool a299;bool a300;bool a301;bool a302;bool a303;bool a304;bool a305;bool a306;bool a307;bool a308;bool a309;bool a310;bool a311;bool a312;bool a313;bool a314;bool a315;bool a316;bool a317;bool a318;bool a319;bool a320;bool a321;bool a322;bool a323;bool a324;bool a325;bool a326;bool a327;bool a328;bool a329;bool a330;bool a331;bool a332;bool a333;bool a334;bool a335;bool a336;bool a337;bool a338;bool a339;bool a340;bool a341;bool a342;bool a343;bool a344;bool a345;bool a346;bool a347;bool a348;bool a349;bool a350;bool a351;bool a352;bool a353;bool a354;bool a355;bool a356;bool a357;bool a358;bool a359;bool a360;bool a361;bool a362;bool a363;bool a364;bool a365;bool a366;bool a367;bool a368;bool a369;bool a370;bool a371;bool a372;bool a373;bool a374;bool a375;bool a376;bool a377;bool a378;bool a379;bool a380;bool a381;bool a382;bool a383;bool a384;bool a385;bool a386;bool a387;bool a388;bool a389;bool a390;bool a391;bool a392;bool a393;bool a394;bool a395;bool a396;bool a397;bool a398;bool a399;bool a400;bool a401;bool a402;bool a403;bool a404;bool a405;bool a406;bool a407;bool a408;bool a409;bool a410;bool a411;bool a412;bool a413;bool a414;bool a415;bool a416;bool a417;bool a418;bool a419;bool a420;bool a421;bool a422;bool a423;bool a424;bool a425;bool a426;bool a427;bool a428;bool a429;bool a430;bool a431;bool a432;bool a433;bool a434;bool a435;bool a436;bool a437;bool a438;bool a439;bool a440;bool a441;bool a442;bool a443;bool a444;bool a445;bool a446;bool a447;bool a448;bool a449;bool a450;bool a451;bool a452;bool a453;bool a454;bool a455;bool a456;bool a457;bool a458;bool a459;bool a460;bool a461;bool a462;bool a463;bool a464;bool a465;bool a466;bool a467;bool a468;bool a469;bool a470;bool a471;bool a472;bool a473;bool a474;bool a475;bool a476;bool a477;bool a478;bool a479;bool a480;bool a481;bool a482;bool a483;bool a484;bool a485;bool a486;bool a487;bool a488;bool a489;bool a490;bool a491;bool a492;bool a493;bool a494;bool a495;bool a496;bool a497;bool a498;bool a499;bool a500;bool a501;bool a502;bool a503;bool a504;bool a505;bool a506;bool a507;bool a508;bool a509;bool a510;bool a511;bool a512;bool a513;bool a514;bool a515;bool a516;bool a517;bool a518;bool a519;bool a520;bool a521;bool a522;bool a523;bool a524;bool a525;bool a526;bool a527;bool a528;bool a529;bool a530;bool a531;bool a532;bool a533;bool a534;bool a535;bool a536;bool a537;bool a538;bool a539;bool a540;bool a541;bool a542;bool a543;bool a544;bool a545;bool a546;bool a547;bool a548;bool a549;bool a550;bool a551;bool a552;bool a553;bool a554;bool a555;bool a556;bool a557;bool a558;bool a559;bool a560;bool a561;bool a562;bool a563;bool a564;bool a565;bool a566;bool a567;bool a568;bool a569;bool a570;bool a571;bool a572;bool a573;bool a574;bool a575;bool a576;bool a577;bool a578;bool a579;bool a580;bool a581;bool a582;bool a583;bool a584;bool a585;bool a586;bool a587;bool a588;bool a589;bool a590;bool a591;bool a592;bool a593;bool a594;bool a595;bool a596;bool a597;bool a598;bool a599;bool a600;bool a601;bool a602;bool a603;bool a604;bool a605;bool a606;bool a607;bool a608;bool a609;bool a610;bool a611;bool a612;bool a613;bool a614;bool a615;bool a616;bool a617;bool a618;bool a619;bool a620;bool a621;bool a622;bool a623;bool a624;bool a625;bool a626;bool a627;bool a628;bool a629;bool a630;bool a631;bool a632;bool a633;bool a634;bool a635;bool a636;bool a637;bool a638;bool a639;bool a640;bool a641;bool a642;bool a643;bool a644;bool a645;bool a646;bool a647;bool a648;bool a649;bool a650;bool a651;bool a652;bool a653;bool a654;bool a655;bool a656;bool a657;bool a658;bool a659;bool a660;bool a661;bool a662;bool a663;bool a664;bool a665;bool a666;bool a667;bool a668;bool a669;bool a670;bool a671;bool a672;bool a673;bool a674;bool a675;bool a676;bool a677;bool a678;bool a679;bool a680;bool a681;bool a682;bool a683;bool a684;bool a685;bool a686;bool a687;bool a688;bool a689;bool a690;bool a691;bool a692;bool a693;bool a694;bool a695;bool a696;bool a697;bool a698;bool a699;bool a700;bool a701;bool a702;bool a703;bool a704;bool a705;bool a706;bool a707;bool a708;bool a709;bool a710;bool a711;bool a712;bool a713;bool a714;bool a715;bool a716;bool a717;bool a718;bool a719;bool a720;bool a721;bool a722;bool a723;bool a724;bool a725;bool a726;bool a727;bool a728;bool a729;bool a730;bool a731;bool a732;bool a733;bool a734;bool a735;bool a736;bool a737;bool a738;bool a739;bool a740;bool a741;bool a742;bool a743;bool a744;bool a745;bool a746;bool a747;bool a748;bool a749;bool a750;bool a751;bool a752;bool a753;bool a754;bool a755;bool a756;bool a757;bool a758;bool a759;bool a760;bool a761;bool a762;bool a763;bool a764;bool a765;bool a766;bool a767;bool a768;bool a769;bool a770;bool a771;bool a772;bool a773;bool a774;bool a775;bool a776;bool a777;bool a778;bool a779;bool a780;bool a781;bool a782;bool a783;bool a784;bool a785;bool a786;bool a787;bool a788;bool a789;bool a790;bool a791;bool a792;bool a793;bool a794;bool a795;bool a796;bool a797;bool a798;bool a799;bool a800;bool a801;bool a802;bool a803;bool a804;bool a805;bool a806;bool a807;bool a808;bool a809;bool a810;bool a811;bool a812;bool a813;bool a814;bool a815;bool a816;bool a817;bool a818;bool a819;bool a820;bool a821;bool a822;bool a823;bool a824;bool a825;bool a826;bool a827;bool a828;bool a829;bool a830;bool a831;bool a832;bool a833;bool a834;bool a835;bool a836;bool a837;bool a838;bool a839;bool a840;bool a841;bool a842;bool a843;bool a844;bool a845;bool a846;bool a847;bool a848;bool a849;bool a850;bool a851;bool a852;bool a853;bool a854;bool a855;bool a856;bool a857;bool a858;bool a859;bool a860;bool a861;bool a862;bool a863;bool a864;bool a865;bool a866;bool a867;bool a868;bool a869;bool a870;bool a871;bool a872;bool a873;bool a874;bool a875;bool a876;bool a877;bool a878;bool a879;bool a880;bool a881;bool a882;bool a883;bool a884;bool a885;bool a886;bool a887;bool a888;bool a889;bool a890;bool a891;bool a892;bool a893;bool a894;bool a895;bool a896;bool a897;bool a898;bool a899;bool a900;bool a901;bool a902;bool a903;bool a904;bool a905;bool a906;bool a907;bool a908;bool a909;bool a910;bool a911;bool a912;bool a913;bool a914;bool a915;bool a916;bool a917;bool a918;bool a919;bool a920;bool a921;bool a922;bool a923;bool a924;bool a925;bool a926;bool a927;bool a928;bool a929;bool a930;bool a931;bool a932;bool a933;bool a934;bool a935;bool a936;bool a937;bool a938;bool a939;bool a940;bool a941;bool a942;bool a943;bool a944;bool a945;bool a946;bool a947;bool a948;bool a949;bool a950;bool a951;bool a952;bool a953;bool a954;bool a955;bool a956;bool a957;bool a958;bool a959;bool a960;bool a961;bool a962;bool a963;bool a964;bool a965;bool a966;bool a967;bool a968;bool a969;bool a970;bool a971;bool a972;bool a973;bool a974;bool a975;bool a976;bool a977;bool a978;bool a979;bool a980;bool a981;bool a982;bool a983;bool a984;bool a985;bool a986;bool a987;bool a988;bool a989;bool a990;bool a991;bool a992;bool a993;bool a994;bool a995;bool a996;bool a997;bool a998;bool a999;bool a1000;
    // clang-format on
};
}  // namespace

TEST(Reflection, BigStructHandling)
{
    {
        constexpr auto RESULT = []()
        {
            FieldCount100 instance{};
            std::size_t counter = 0;
            reflection::for_each_field(instance,
                                       [&]<typename T>(const std::string_view& /*name*/, const T&)
                                       { counter++; });
            return counter;
        }();

        static_assert(100 == RESULT);
    }

    {
#if !defined(FIXED_CONTAINERS_CLANG_TIDY_RUNNING) || FIXED_CONTAINERS_CLANG_TIDY_RUNNING == 0
        constexpr auto RESULT = []()
        {
            FieldCount1000 instance{};
            std::size_t counter = 0;
            reflection::for_each_field(instance,
                                       [&]<typename T>(const std::string_view& /*name*/, const T&)
                                       { counter++; });
            return counter;
        }();

        static_assert(1000 == RESULT);
#endif
    }
}

}  // namespace fixed_containers

#endif
