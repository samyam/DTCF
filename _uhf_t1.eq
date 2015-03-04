
{
 range Oa, Ob = 10.0;
 range Va, Vb = 100.0;
 index h1a, h2a, h3a, h4a = Oa;
 index h1b, h2b, h3b, h4b = Ob;
 index p1a, p2a, p3a, p4a = Va;
 index p1b, p2b, p3b, p4b = Vb;
 array fa_oo([Oa][Oa]), fa_ov([Oa][Va]), fa_vo([Va][Oa]), fa_vv([Va][Va]), fb_oo([Ob][Ob]);
 array fb_ov([Ob][Vb]), fb_vo([Vb][Ob]), fb_vv([Vb][Vb]), vaa_oovo([Oa,Oa][Va,Oa]:(0,1)), vaa_oovv([Oa,Oa][Va,Va]:(0,1)(2,3));
 array vaa_vovo([Va,Oa][Va,Oa]), vaa_vovv([Va,Oa][Va,Va]:(2,3)), vbb_oovo([Ob,Ob][Vb,Ob]:(0,1)), vbb_oovv([Ob,Ob][Vb,Vb]:(0,1)(2,3)), vbb_vovo([Vb,Ob][Vb,Ob]);
 array vbb_vovv([Vb,Ob][Vb,Vb]:(2,3)), vab_ooov([Oa,Ob][Oa,Vb]), vab_oovo([Oa,Ob][Va,Ob]), vab_oovv([Oa,Ob][Va,Vb]), vab_ovvo([Oa,Vb][Va,Ob]);
 array vab_ovvv([Oa,Vb][Va,Vb]), vab_voov([Va,Ob][Oa,Vb]), vab_vovv([Va,Ob][Va,Vb]), ta_vo([Va][Oa]), tb_vo([Vb][Ob]);
 array taa_vvoo([Va,Va][Oa,Oa]:(0,1)(2,3)), tbb_vvoo([Vb,Vb][Ob,Ob]:(0,1)(2,3)), tab_vvoo([Va,Vb][Oa,Ob]), ra_vo([Va][Oa]), rb_vo([Vb][Ob]);
 array _a587([Oa,Ob][Vb,Oa]), _a3988([Va][Va]), _a3989([Va][Va]), _a3990([Va][Oa]), _a287([Va][Oa]);
 array _a371([Oa][Va]), _a377([Ob][Vb]), _a708([Oa][Oa]), _a709([Oa][Oa]), _a365([Ob][Vb]);
 array _a724([Ob][Vb]), _a588([Oa,Ob][Vb,Oa]), _a589([Va][Oa]), _a301([Oa,Oa][Va,Oa]:(0,1)), _a685([Oa,Oa][Va,Oa]:(0,1));
 array _a686([Va][Oa]), _a1031([Oa][Va]), _a3880([Oa][Oa]), _a3881([Oa][Oa]), _a3843([Oa][Oa]);
 array _a501([Va][Oa]), _a623([Va,Vb][Oa,Ob]), _a3844([Oa][Oa]), _a3845([Oa][Oa]), _a3945([Oa][Oa]);
 array _a624([Va,Vb][Oa,Ob]), _a625([Va][Oa]), _a502([Va][Oa]), _a710([Oa][Oa]), _a383([Oa][Va]);
 array _a2068([Oa][Va]), _a2069([Va][Oa]), _a3457([Oa][Oa]), _a3463([Oa][Oa]), _a3469([Oa][Oa]);
 array _a3969([Oa][Oa]), _a3993([Oa][Oa]), _a3994([Va][Oa]), _a2925([Ob][Vb]), _a2926([Va][Oa]);
 _a587[h2a,h1b,p1b,h1a] = (vab_oovv[h2a,h1b,p2a,p1b] * ta_vo[p2a,h1a])
# 20000000.0	10.0*10.0*10.0*100.0*100.0*2.0

 _a3988[p1a,p2a] = (vaa_vovv[p1a,h2a,p2a,p3a] * ta_vo[p3a,h2a])
# 20000000.0	100.0*100.0*10.0*100.0*2.0

 _a3989[p1a,p2a] = (_a3988[p1a,p2a] + fa_vv[p1a,p2a])
# 20000.0	100.0*100.0*2

 _a3990[p1a,h1a] = (ta_vo[p2a,h1a] * _a3989[p1a,p2a])
# 200000.0	10.0*100.0*100.0*2.0

 _a287[p1a,h1a] = (vaa_vovv[p1a,h2a,p2a,p3a] * taa_vvoo[p2a,p3a,h1a,h2a])
# 101000000.0	100.0*10.0*10.0*101.0*100.0/2.0*2.0

 _a371[h2a,p2a] = (vab_oovv[h2a,h1b,p2a,p1b] * tb_vo[p1b,h1b])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 _a377[h1b,p1b] = (vab_oovv[h2a,h1b,p2a,p1b] * ta_vo[p2a,h2a])
# 2000000.0	10.0*100.0*10.0*100.0*2.0

 _a708[h2a,h1a] = (vab_oovv[h2a,h1b,p2a,p1b] * tab_vvoo[p2a,p1b,h1a,h1b])
# 20000000.0	10.0*10.0*10.0*100.0*100.0*2.0

 _a709[h2a,h1a] = (vaa_oovv[h2a,h3a,p2a,p3a] * taa_vvoo[p2a,p3a,h1a,h3a])
# 10100000.0	10.0*10.0*10.0*101.0*100.0/2.0*2.0

 _a365[h2b,p2b] = (vbb_oovv[h1b,h2b,p1b,p2b] * tb_vo[p1b,h1b])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 _a724[h2b,p2b] = (_a365[h2b,p2b] + _a377[h2b,p2b])
# 2000.0	100.0*10.0*2

 _a588[h2a,h1b,p1b,h1a] = (_a587[h2a,h1b,p1b,h1a] + vab_ooov[h2a,h1b,h1a,p1b])
# 200000.0	10.0*10.0*10.0*100.0*2

 _a589[p1a,h1a] = (tab_vvoo[p1a,p1b,h2a,h1b] * _a588[h2a,h1b,p1b,h1a])
# 20000000.0	100.0*10.0*100.0*10.0*10.0*2.0

 _a301[h2a,h3a,p3a,h1a] = (vaa_oovv[h2a,h3a,p2a,p3a] * ta_vo[p2a,h1a])
# 11000000.0	10.0*100.0*11.0*10.0/2.0*100.0*2.0

 _a685[h2a,h3a,p3a,h1a] = (_a301[h2a,h3a,p3a,h1a] + -vaa_oovo[h2a,h3a,p3a,h1a])
# 110000.0	10.0*100.0*11.0*10.0/2.0*2

 _a686[p1a,h1a] = (taa_vvoo[p1a,p3a,h2a,h3a] * _a685[h2a,h3a,p3a,h1a])
# 11000000.0	100.0*10.0*100.0*11.0*10.0/2.0*2.0

 _a1031[h2a,p2a] = (_a371[h2a,p2a] + fa_ov[h2a,p2a])
# 2000.0	100.0*10.0*2

 _a3880[h2a,h1a] = (fa_ov[h2a,p2a] * ta_vo[p2a,h1a])
# 20000.0	10.0*10.0*100.0*2.0

 _a3881[h2a,h1a] = (_a3880[h2a,h1a] + fa_oo[h2a,h1a])
# 200.0	10.0*10.0*2

 _a3843[h2a,h1a] = (vab_ooov[h2a,h1b,h1a,p1b] * tb_vo[p1b,h1b])
# 200000.0	10.0*10.0*10.0*100.0*2.0

 _a501[p1a,h1a] = (vab_voov[p1a,h1b,h1a,p1b] * tb_vo[p1b,h1b])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 _a623[p2a,p1b,h1a,h1b] = (ta_vo[p2a,h1a] * tb_vo[p1b,h1b])
# 2000000.0	10.0*100.0*100.0*10.0**2.0

 _a3844[h2a,h1a] = (vaa_oovo[h2a,h3a,p2a,h1a] * ta_vo[p2a,h3a])
# 200000.0	10.0*10.0*10.0*100.0*2.0

 _a3845[h2a,h1a] = (_a3843[h2a,h1a] + -_a3844[h2a,h1a])
# 200.0	10.0*10.0*2

 _a3945[h2a,h1a] = (_a3845[h2a,h1a] + _a3881[h2a,h1a])
# 200.0	10.0*10.0*2

 _a624[p2a,p1b,h1a,h1b] = (_a623[p2a,p1b,h1a,h1b] + tab_vvoo[p2a,p1b,h1a,h1b])
# 2000000.0	10.0*100.0*100.0*10.0*2

 _a625[p1a,h1a] = (vab_vovv[p1a,h1b,p2a,p1b] * _a624[p2a,p1b,h1a,h1b])
# 200000000.0	100.0*10.0*10.0*100.0*100.0*2.0

 _a502[p1a,h1a] = (vaa_vovo[p1a,h2a,p2a,h1a] * ta_vo[p2a,h2a])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 _a710[h2a,h1a] = ((2.0 * _a708[h2a,h1a]) + _a709[h2a,h1a])
# 300.0	(10.0*10.0*2+100.0)

 _a383[h3a,p3a] = (vaa_oovv[h2a,h3a,p2a,p3a] * ta_vo[p2a,h2a])
# 2000000.0	10.0*100.0*10.0*100.0*2.0

 _a2068[h3a,p3a] = (_a1031[h3a,p3a] + _a383[h3a,p3a])
# 2000.0	100.0*10.0*2

 _a2069[p1a,h1a] = (taa_vvoo[p1a,p3a,h1a,h3a] * _a2068[h3a,p3a])
# 2000000.0	100.0*10.0*100.0*10.0*2.0

 _a3457[h2a,h1a] = (ta_vo[p2a,h1a] * _a371[h2a,p2a])
# 20000.0	10.0*10.0*100.0*2.0

 _a3463[h2a,h1a] = (ta_vo[p3a,h3a] * _a301[h2a,h3a,p3a,h1a])
# 200000.0	10.0*10.0*100.0*10.0*2.0

 _a3469[h2a,h1a] = (_a3457[h2a,h1a] + _a3463[h2a,h1a])
# 200.0	10.0*10.0*2

 _a3969[h2a,h1a] = ((2.0 * _a3469[h2a,h1a]) + _a710[h2a,h1a])
# 300.0	(10.0*10.0*2+100.0)

 _a3993[h2a,h1a] = ((2.0 * _a3945[h2a,h1a]) + _a3969[h2a,h1a])
# 300.0	(10.0*10.0*2+100.0)

 _a3994[p1a,h1a] = (ta_vo[p1a,h2a] * _a3993[h2a,h1a])
# 20000.0	100.0*10.0*10.0*2.0

 _a2925[h1b,p1b] = (_a724[h1b,p1b] + fb_ov[h1b,p1b])
# 2000.0	100.0*10.0*2

 _a2926[p1a,h1a] = (tab_vvoo[p1a,p1b,h1a,h1b] * _a2925[h1b,p1b])
# 2000000.0	100.0*10.0*100.0*10.0*2.0

 ra_vo[p1a,h1a] = (fa_vo[p1a,h1a] + (0.5 * _a287[p1a,h1a]) + -_a589[p1a,h1a] + _a625[p1a,h1a] + (-0.5 * _a686[p1a,h1a]) + _a501[p1a,h1a] + -_a502[p1a,h1a] + _a2069[p1a,h1a] + _a2926[p1a,h1a] + _a3990[p1a,h1a] + (-0.5 * _a3994[p1a,h1a]))
# 14000.0	(100.0*10.0*11+3000.0)

 array _a4156([Ob][Ob]), _a288([Vb][Ob]), _a3996([Vb][Ob]), _a4094([Oa,Ob][Va,Ob]), _a4095([Oa,Ob][Va,Ob]);
 array _a4096([Vb][Ob]), _a6407([Vb][Vb]), _a6408([Vb][Vb]), _a6409([Vb][Ob]), _a4155([Ob][Ob]);
 array _a6263([Ob][Ob]), _a6299([Ob][Ob]), _a6262([Ob][Ob]), _a6264([Ob][Ob]), _a505([Vb][Ob]);
 array _a4157([Ob][Ob]), _a5882([Ob][Ob]), _a307([Ob,Ob][Vb,Ob]:(0,1)), _a5879([Ob][Ob]), _a4132([Ob,Ob][Vb,Ob]:(0,1));
 array _a4133([Vb][Ob]), _a5888([Ob][Ob]), _a6388([Ob][Ob]), _a5421([Vb][Ob]), _a4807([Vb][Ob]);
 array _a6300([Ob][Ob]), _a6364([Ob][Ob]), _a6412([Ob][Ob]), _a6413([Vb][Ob]), _a506([Vb][Ob]);
 _a4156[h2b,h1b] = (vbb_oovv[h2b,h3b,p2b,p3b] * tbb_vvoo[p2b,p3b,h1b,h3b])
# 10100000.0	10.0*10.0*10.0*101.0*100.0/2.0*2.0

 _a288[p1b,h1b] = (vbb_vovv[p1b,h2b,p2b,p3b] * tbb_vvoo[p2b,p3b,h1b,h2b])
# 101000000.0	100.0*10.0*10.0*101.0*100.0/2.0*2.0

 _a3996[p1b,h1b] = (vab_ovvv[h1a,p1b,p1a,p2b] * _a624[p1a,p2b,h1a,h1b])
# 200000000.0	100.0*10.0*10.0*100.0*100.0*2.0

 _a4094[h1a,h2b,p1a,h1b] = (vab_oovv[h1a,h2b,p1a,p2b] * tb_vo[p2b,h1b])
# 20000000.0	10.0*10.0*100.0*10.0*100.0*2.0

 _a4095[h1a,h2b,p1a,h1b] = (_a4094[h1a,h2b,p1a,h1b] + vab_oovo[h1a,h2b,p1a,h1b])
# 200000.0	10.0*10.0*100.0*10.0*2

 _a4096[p1b,h1b] = (tab_vvoo[p1a,p1b,h1a,h2b] * _a4095[h1a,h2b,p1a,h1b])
# 20000000.0	100.0*10.0*100.0*10.0*10.0*2.0

 _a6407[p1b,p2b] = (vbb_vovv[p1b,h2b,p2b,p3b] * tb_vo[p3b,h2b])
# 20000000.0	100.0*100.0*10.0*100.0*2.0

 _a6408[p1b,p2b] = (_a6407[p1b,p2b] + fb_vv[p1b,p2b])
# 20000.0	100.0*100.0*2

 _a6409[p1b,h1b] = (tb_vo[p2b,h1b] * _a6408[p1b,p2b])
# 200000.0	10.0*100.0*100.0*2.0

 _a4155[h2b,h1b] = (vab_oovv[h1a,h2b,p1a,p2b] * tab_vvoo[p1a,p2b,h1a,h1b])
# 20000000.0	10.0*10.0*10.0*100.0*100.0*2.0

 _a6263[h2b,h1b] = (vab_oovo[h1a,h2b,p1a,h1b] * ta_vo[p1a,h1a])
# 200000.0	10.0*10.0*10.0*100.0*2.0

 _a6299[h2b,h1b] = (fb_ov[h2b,p2b] * tb_vo[p2b,h1b])
# 20000.0	10.0*10.0*100.0*2.0

 _a6262[h2b,h1b] = (vbb_oovo[h2b,h3b,p2b,h1b] * tb_vo[p2b,h3b])
# 200000.0	10.0*10.0*10.0*100.0*2.0

 _a6264[h2b,h1b] = (_a6262[h2b,h1b] + -_a6263[h2b,h1b])
# 200.0	10.0*10.0*2

 _a505[p1b,h1b] = (vbb_vovo[p1b,h2b,p2b,h1b] * tb_vo[p2b,h2b])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 _a4157[h2b,h1b] = ((2.0 * _a4155[h2b,h1b]) + _a4156[h2b,h1b])
# 300.0	(10.0*10.0*2+100.0)

 _a5882[h2b,h1b] = (tb_vo[p2b,h1b] * _a377[h2b,p2b])
# 20000.0	10.0*10.0*100.0*2.0

 _a307[h2b,h3b,p3b,h1b] = (vbb_oovv[h2b,h3b,p2b,p3b] * tb_vo[p2b,h1b])
# 11000000.0	11.0*10.0/2.0*100.0*10.0*100.0*2.0

 _a5879[h2b,h1b] = (tb_vo[p3b,h3b] * _a307[h2b,h3b,p3b,h1b])
# 200000.0	10.0*10.0*100.0*10.0*2.0

 _a4132[h2b,h3b,p3b,h1b] = (_a307[h2b,h3b,p3b,h1b] + -vbb_oovo[h2b,h3b,p3b,h1b])
# 110000.0	11.0*10.0/2.0*100.0*10.0*2

 _a4133[p1b,h1b] = (tbb_vvoo[p1b,p3b,h2b,h3b] * _a4132[h2b,h3b,p3b,h1b])
# 11000000.0	100.0*10.0*100.0*11.0*10.0/2.0*2.0

 _a5888[h2b,h1b] = (_a5879[h2b,h1b] + _a5882[h2b,h1b])
# 200.0	10.0*10.0*2

 _a6388[h2b,h1b] = (_a4157[h2b,h1b] + (2.0 * _a5888[h2b,h1b]))
# 300.0	(10.0*10.0*2+100.0)

 _a5421[p1b,h1b] = (tbb_vvoo[p1b,p2b,h1b,h2b] * _a2925[h2b,p2b])
# 2000000.0	100.0*10.0*100.0*10.0*2.0

 _a4807[p1b,h1b] = (tab_vvoo[p2a,p1b,h2a,h1b] * _a2068[h2a,p2a])
# 2000000.0	100.0*10.0*100.0*10.0*2.0

 _a6300[h2b,h1b] = (_a6299[h2b,h1b] + fb_oo[h2b,h1b])
# 200.0	10.0*10.0*2

 _a6364[h2b,h1b] = (_a6264[h2b,h1b] + -_a6300[h2b,h1b])
# 200.0	10.0*10.0*2

 _a6412[h2b,h1b] = ((2.0 * _a6364[h2b,h1b]) + -_a6388[h2b,h1b])
# 300.0	(10.0*10.0*2+100.0)

 _a6413[p1b,h1b] = (tb_vo[p1b,h2b] * _a6412[h2b,h1b])
# 20000.0	100.0*10.0*10.0*2.0

 _a506[p1b,h1b] = (vab_ovvo[h1a,p1b,p1a,h1b] * ta_vo[p1a,h1a])
# 2000000.0	100.0*10.0*10.0*100.0*2.0

 rb_vo[p1b,h1b] = (fb_vo[p1b,h1b] + (0.5 * _a288[p1b,h1b]) + _a3996[p1b,h1b] + -_a4096[p1b,h1b] + (-0.5 * _a4133[p1b,h1b]) + -_a505[p1b,h1b] + _a506[p1b,h1b] + _a4807[p1b,h1b] + _a5421[p1b,h1b] + _a6409[p1b,h1b] + (0.5 * _a6413[p1b,h1b]))
# 14000.0	(100.0*10.0*11+3000.0)

}
# TOTAL COST=856619400.0
