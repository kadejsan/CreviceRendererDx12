#include "stdafx.h"
#include "RayTracedAmbientOcclusion.h"
#include "MathHelper.h"
#include "Renderer.h"
#include "UIContext.h"

#pragma region AOSamples

float3 aoSamples[360] = {
	float3(-0.573172f, -0.532465f, 0.62286f),
	float3(0.587828f, 0.380267f, 0.714042f),
	float3(-0.857373f, -0.448367f, 0.252742f),
	float3(0.322927f, -0.208062f, 0.92327f),
	float3(-0.770474f, 0.378665f, 0.512818f),
	float3(0.367633f, -0.868896f, 0.33146f),
	float3(-0.073854f, 0.628146f, 0.774583f),
	float3(-0.383834f, -0.922466f, 0.0415685f),
	float3(-0.139787f, -0.145552f, 0.979425f),
	float3(0.825291f, 0.495564f, 0.27076f),
	float3(-0.894489f, -0.061784f, 0.4428f),
	float3(0.600867f, 0.058068f, 0.797237f),
	float3(-0.693786f, 0.696058f, 0.184836f),
	float3(0.495962f, -0.566117f, 0.658432f),
	float3(-0.272272f, 0.830458f, 0.486012f),
	float3(-0.100632f, -0.788655f, 0.606545f),
	float3(-0.629072f, -0.276978f, 0.726328f),
	float3(0.064968f, -0.439588f, 0.895847f),
	float3(-0.340094f, -0.656093f, 0.673705f),
	float3(0.934086f, -0.248247f, 0.256626f),
	float3(-0.67346f, 0.11431f, 0.730332f),
	float3(0.759222f, -0.60907f, 0.229381f),
	float3(0.562448f, 0.821412f, 0.0945225f),
	float3(0.117007f, -0.9522f, 0.282178f),
	float3(0.23493f, 0.591106f, 0.771623f),
	float3(0.014093f, 0.243143f, 0.969888f),
	float3(-0.426217f, 0.465168f, 0.775859f),
	float3(-0.028122f, 0.950881f, 0.308277f),
	float3(-0.37139f, 0.017965f, 0.928303f),
	float3(0.879943f, 0.09586f, 0.465308f),
	float3(0.260057f, 0.837749f, 0.480153f),
	float3(0.172604f, -0.681421f, 0.711248f),
	float3(-0.209746f, 0.323099f, 0.922829f),
	float3(-0.360208f, -0.373921f, 0.854654f),
	float3(0.620504f, -0.268533f, 0.736794f),
	float3(0.248378f, 0.087706f, 0.964684f),
	float3(-0.111673f, -0.805692f, 0.581713f),
	float3(-0.092171f, 0.198186f, 0.975821f),
	float3(-0.41279f, -0.006234f, 0.910805f),
	float3(0.386095f, 0.741015f, 0.549388f),
	float3(0.243681f, -0.902202f, 0.355881f),
	float3(-0.966973f, 0.05069f, 0.249787f),
	float3(-0.451944f, -0.391673f, 0.80146f),
	float3(0.470578f, 0.213208f, 0.856212f),
	float3(0.730174f, -0.629536f, 0.265576f),
	float3(-0.505434f, 0.453231f, 0.734247f),
	float3(-0.847213f, -0.420726f, 0.324376f),
	float3(0.88622f, -0.135896f, 0.442884f),
	float3(0.472295f, -0.378033f, 0.796259f),
	float3(-0.433324f, 0.899029f, 0.0630644f),
	float3(-0.512198f, -0.84487f, 0.154428f),
	float3(0.33882f, -0.062101f, 0.938799f),
	float3(-0.00555f, -0.425101f, 0.905129f),
	float3(-0.20188f, 0.619922f, 0.758249f),
	float3(-0.064827f, 0.985998f, 0.153641f),
	float3(0.497291f, -0.855726f, 0.14295f),
	float3(0.923471f, 0.129255f, 0.36124f),
	float3(-0.353302f, -0.681945f, 0.640413f),
	float3(0.547634f, 0.828596f, 0.1163f),
	float3(0.452132f, -0.217132f, 0.865119f),
	float3(0.601841f, 0.335957f, 0.724514f),
	float3(-0.662233f, -0.374095f, 0.649231f),
	float3(-0.830721f, 0.36586f, 0.419582f),
	float3(0.134286f, -0.458476f, 0.878503f),
	float3(0.666723f, -0.493933f, 0.558131f),
	float3(-0.357254f, 0.138589f, 0.923668f),
	float3(-0.069659f, 0.765169f, 0.64005f),
	float3(-0.164765f, -0.54802f, 0.820077f),
	float3(-0.342f, 0.43318f, 0.833901f),
	float3(-0.269501f, -0.381071f, 0.884395f),
	float3(0.112766f, 0.241213f, 0.963898f),
	float3(0.253631f, 0.121867f, 0.959594f),
	float3(0.087463f, -0.98784f, 0.128539f),
	float3(-0.248589f, 0.827489f, 0.503453f),
	float3(-0.894474f, -0.173916f, 0.411909f),
	float3(0.241316f, 0.860883f, 0.447936f),
	float3(-0.301417f, -0.940516f, 0.156772f),
	float3(-0.133819f, 0.458456f, 0.878584f),
	float3(-0.59478f, 0.283413f, 0.752272f),
	float3(0.602431f, 0.573521f, 0.555113f),
	float3(0.887774f, -0.383196f, 0.254987f),
	float3(0.294239f, 0.284313f, 0.912463f),
	float3(-0.416595f, 0.721009f, 0.55371f),
	float3(0.542035f, 0.051508f, 0.838776f),
	float3(0.42716f, -0.685994f, 0.589022f),
	float3(-0.181728f, 0.01092f, 0.983288f),
	float3(0.70661f, 0.114861f, 0.698219f),
	float3(0.292003f, -0.34892f, 0.890499f),
	float3(-0.46086f, -0.174902f, 0.870067f),
	float3(-0.005891f, -0.249559f, 0.968342f),
	float3(-0.685888f, 0.674723f, 0.272592f),
	float3(0.10816f, -0.640948f, 0.759926f),
	float3(0.793963f, 0.589964f, 0.146851f),
	float3(-0.70433f, -0.201913f, 0.680552f),
	float3(0.841996f, 0.399802f, 0.362217f),
	float3(-0.731786f, 0.037678f, 0.680492f),
	float3(0.745759f, -0.314864f, 0.587115f),
	float3(-0.211147f, -0.243536f, 0.946629f),
	float3(-0.801454f, 0.523342f, 0.289456f),
	float3(0.162043f, -0.186723f, 0.968956f),
	float3(0.347945f, -0.52351f, 0.777735f),
	float3(0.00078f, 0.522231f, 0.852804f),
	float3(0.109733f, 0.7145f, 0.690976f),
	float3(0.40126f, 0.517453f, 0.755799f),
	float3(-0.644107f, -0.539325f, 0.542453f),
	float3(0.215646f, 0.44983f, 0.866689f),
	float3(0.650355f, -0.164203f, 0.741671f),
	float3(0.081457f, 0.066312f, 0.994468f),
	float3(-0.51502f, -0.28679f, 0.807778f),
	float3(0.157959f, -0.594221f, 0.788638f),
	float3(0.026443f, 0.409725f, 0.911826f),
	float3(-0.189282f, -0.962532f, 0.194176f),
	float3(0.963359f, -0.195712f, 0.183402f),
	float3(0.457808f, 0.688883f, 0.562007f),
	float3(-0.811535f, -0.003014f, 0.584296f),
	float3(-0.908651f, 0.302434f, 0.287901f),
	float3(-0.474748f, 0.250181f, 0.843815f),
	float3(-0.491032f, -0.839682f, 0.231995f),
	float3(0.771686f, -0.629872f, 0.0881016f),
	float3(0.872363f, 0.397718f, 0.284259f),
	float3(-0.388794f, -0.019206f, 0.921125f),
	float3(0.001837f, 0.020697f, 0.999784f),
	float3(-0.632117f, 0.523246f, 0.571526f),
	float3(-0.778495f, -0.59278f, 0.206294f),
	float3(0.460797f, -0.765526f, 0.449039f),
	float3(0.58083f, 0.161932f, 0.797756f),
	float3(-0.180654f, -0.607449f, 0.773544f),
	float3(0.513693f, -0.167325f, 0.8415f),
	float3(-0.421676f, 0.845932f, 0.326479f),
	float3(-0.900007f, -0.364284f, 0.239342f),
	float3(0.264091f, -0.948709f, 0.173802f),
	float3(0.259477f, 0.105395f, 0.959981f),
	float3(-0.295673f, -0.384011f, 0.874707f),
	float3(0.247187f, -0.234391f, 0.940191f),
	float3(-0.142494f, 0.545041f, 0.826212f),
	float3(0.749411f, 0.566827f, 0.342185f),
	float3(-0.751642f, 0.391944f, 0.530485f),
	float3(-0.01888f, -0.973469f, 0.228039f),
	float3(0.488952f, 0.855218f, 0.171838f),
	float3(-0.744475f, -0.447191f, 0.495759f),
	float3(0.16833f, -0.840767f, 0.514564f),
	float3(0.876009f, -0.480771f, 0.0383079f),
	float3(0.581396f, -0.705249f, 0.405712f),
	float3(0.264532f, -0.411582f, 0.872137f),
	float3(-0.200065f, 0.895941f, 0.396565f),
	float3(-0.493317f, 0.446332f, 0.74661f),
	float3(-0.41263f, -0.16412f, 0.895992f),
	float3(0.85533f, -0.073753f, 0.512807f),
	float3(-0.355549f, -0.908499f, 0.219578f),
	float3(0.37168f, -0.256708f, 0.892163f),
	float3(0.686929f, 0.344122f, 0.640085f),
	float3(-0.683593f, 0.075293f, 0.725969f),
	float3(-0.288755f, -0.249734f, 0.924258f),
	float3(0.266794f, 0.548329f, 0.792563f),
	float3(-0.70091f, -0.307015f, 0.643791f),
	float3(-0.030425f, -0.545163f, 0.837778f),
	float3(0.698483f, -0.221878f, 0.680361f),
	float3(-0.337681f, 0.334356f, 0.879874f),
	float3(0.060789f, -0.127245f, 0.990007f),
	float3(0.035742f, 0.576255f, 0.816488f),
	float3(-0.516496f, 0.108531f, 0.849384f),
	float3(0.389827f, 0.163983f, 0.90617f),
	float3(-0.676275f, -0.717207f, 0.168126f),
	float3(0.999848f, -0.009693f, 0.0144924f),
	float3(-0.169199f, 0.736135f, 0.655345f),
	float3(-0.513565f, -0.672696f, 0.532664f),
	float3(0.613157f, 0.761857f, 0.208836f),
	float3(-0.211623f, 0.414044f, 0.885315f),
	float3(-0.801634f, 0.536874f, 0.262962f),
	float3(-0.06817f, 0.988983f, 0.131398f),
	float3(-0.562874f, -0.109299f, 0.819284f),
	float3(0.408448f, -0.909855f, 0.0730348f),
	float3(0.404144f, -0.583296f, 0.70458f),
	float3(0.29895f, 0.352669f, 0.886709f),
	float3(-0.962923f, 0.048307f, 0.265416f),
	float3(0.597607f, 0.537985f, 0.594507f),
	float3(-0.163459f, -0.011066f, 0.986488f),
	float3(-0.943801f, -0.136299f, 0.301102f),
	float3(0.564877f, 0.39128f, 0.726508f),
	float3(0.116372f, 0.199205f, 0.973024f),
	float3(-0.135207f, -0.799951f, 0.584634f),
	float3(0.596849f, 0.013229f, 0.802244f),
	float3(0.285713f, -0.068053f, 0.955896f),
	float3(-0.603045f, 0.727626f, 0.326951f),
	float3(0.124742f, -0.332911f, 0.934671f),
	float3(-0.101185f, -0.405175f, 0.908622f),
	float3(0.863899f, -0.338789f, 0.372694f),
	float3(0.104325f, 0.714328f, 0.691991f),
	float3(-0.062139f, -0.265563f, 0.962089f),
	float3(0.287759f, 0.886281f, 0.362906f),
	float3(-0.700598f, -0.114418f, 0.704323f),
	float3(0.608691f, -0.345063f, 0.714442f),
	float3(0.97378f, 0.151037f, 0.170119f),
	float3(-0.604933f, -0.550137f, 0.575678f),
	float3(0.415536f, -0.418477f, 0.807593f),
	float3(0.67441f, -0.518686f, 0.525486f),
	float3(0.052197f, -0.821247f, 0.56818f),
	float3(0.451617f, 0.54084f, 0.709601f),
	float3(0.072125f, 0.865764f, 0.495228f),
	float3(-0.44278f, 0.673523f, 0.591872f),
	float3(-0.308999f, 0.572591f, 0.759381f),
	float3(0.827834f, 0.175739f, 0.532735f),
	float3(-0.353923f, -0.706492f, 0.612868f),
	float3(0.428978f, -0.035364f, 0.902622f),
	float3(0.750334f, 0.050485f, 0.659128f),
	float3(-0.420022f, -0.462855f, 0.780607f),
	float3(-0.087177f, -0.14134f, 0.986115f),
	float3(0.279903f, 0.73218f, 0.62094f),
	float3(-0.635881f, 0.308319f, 0.707527f),
	float3(-0.237762f, 0.210657f, 0.948205f),
	float3(0.30289f, -0.721601f, 0.622535f),
	float3(-0.042474f, 0.715446f, 0.697376f),
	float3(0.167338f, 0.40986f, 0.896668f),
	float3(-0.868885f, 0.164193f, 0.46699f),
	float3(0.025359f, -0.680529f, 0.732282f),
	float3(-0.062392f, 0.248122f, 0.966717f),
	float3(-0.390634f, -0.6522f, 0.649646f),
	float3(0.137677f, -0.546363f, 0.826155f),
	float3(-0.336191f, 0.216193f, 0.916644f),
	float3(0.102813f, 0.386533f, 0.916527f),
	float3(-0.860669f, -0.492747f, 0.128255f),
	float3(0.179158f, -0.033541f, 0.983248f),
	float3(0.853953f, -0.114654f, 0.507562f),
	float3(0.010298f, 0.942333f, 0.334518f),
	float3(-0.029607f, -0.726607f, 0.686415f),
	float3(0.169038f, -0.908303f, 0.382638f),
	float3(-0.95366f, 0.070718f, 0.292458f),
	float3(0.411431f, 0.311704f, 0.856484f),
	float3(-0.924065f, -0.222466f, 0.310826f),
	float3(-0.034844f, -0.193246f, 0.980531f),
	float3(0.480424f, -0.011893f, 0.876956f),
	float3(-0.190246f, 0.729656f, 0.656817f),
	float3(-0.089242f, -0.993831f, 0.0658474f),
	float3(0.460571f, -0.746162f, 0.480746f),
	float3(-0.748474f, 0.277033f, 0.602527f),
	float3(0.943545f, 0.224549f, 0.243517f),
	float3(-0.672377f, -0.086736f, 0.73511f),
	float3(-0.347078f, -0.087383f, 0.933756f),
	float3(0.684807f, -0.311489f, 0.658797f),
	float3(-0.608949f, 0.786797f, 0.100656f),
	float3(-0.375845f, -0.902154f, 0.211799f),
	float3(0.648746f, -0.629388f, 0.427784f),
	float3(-0.789415f, 0.493358f, 0.36527f),
	float3(0.499097f, 0.750078f, 0.433918f),
	float3(-0.532094f, -0.440568f, 0.723032f),
	float3(-0.255664f, -0.438146f, 0.86178f),
	float3(-0.062146f, 0.061693f, 0.996159f),
	float3(-0.200159f, 0.386272f, 0.900406f),
	float3(-0.579263f, -0.812408f, 0.0666911f),
	float3(0.406099f, -0.441059f, 0.800344f),
	float3(-0.446954f, 0.493021f, 0.746433f),
	float3(0.169537f, 0.672549f, 0.720371f),
	float3(0.464456f, 0.875859f, 0.130964f),
	float3(-0.715145f, 0.607413f, 0.345857f),
	float3(0.07613f, -0.996828f, 0.0231972f),
	float3(-0.46812f, -0.547798f, 0.693384f),
	float3(0.826646f, 0.406941f, 0.388659f),
	float3(-0.984976f, -0.081476f, 0.152263f),
	float3(-0.447215f, -0.778893f, 0.439687f),
	float3(-0.420773f, -0.305459f, 0.854193f),
	float3(0.140043f, 0.983766f, 0.112215f),
	float3(-0.959085f, 0.182121f, 0.216767f),
	float3(0.53557f, -0.62069f, 0.572633f),
	float3(-0.253695f, -0.281484f, 0.925422f),
	float3(0.926393f, 0.007891f, 0.376475f),
	float3(-0.916206f, -0.376655f, 0.136739f),
	float3(-0.266703f, -0.945742f, 0.185584f),
	float3(0.297287f, -0.053369f, 0.953295f),
	float3(-0.474479f, 0.873f, 0.112874f),
	float3(-0.65051f, 0.181042f, 0.737605f),
	float3(0.298037f, -0.885616f, 0.356172f),
	float3(-0.331871f, 0.354257f, 0.874279f),
	float3(0.841818f, -0.24546f, 0.48072f),
	float3(0.704161f, -0.501873f, 0.502276f),
	float3(-0.535979f, -0.09316f, 0.839076f),
	float3(0.440766f, 0.116458f, 0.890035f),
	float3(0.309806f, 0.731478f, 0.607421f),
	float3(-0.322001f, 0.464757f, 0.824813f),
	float3(0.56277f, -0.356101f, 0.745977f),
	float3(-0.295348f, 0.036179f, 0.954704f),
	float3(0.534566f, 0.410579f, 0.738691f),
	float3(0.066995f, 0.522813f, 0.849811f),
	float3(0.151187f, -0.680472f, 0.717008f),
	float3(0.051271f, -0.027137f, 0.998316f),
	float3(-0.047559f, 0.764981f, 0.642295f),
	float3(-0.189256f, 0.537003f, 0.822077f),
	float3(0.192415f, -0.422155f, 0.885868f),
	float3(-0.04677f, 0.208916f, 0.976815f),
	float3(-0.561881f, -0.642148f, 0.521474f),
	float3(0.970178f, -0.226543f, 0.0862142f),
	float3(-0.234692f, 0.937908f, 0.255437f),
	float3(0.298317f, 0.403625f, 0.864924f),
	float3(-0.709625f, -0.501003f, 0.495407f),
	float3(0.645027f, 0.695191f, 0.317253f),
	float3(0.316946f, 0.872519f, 0.371828f),
	float3(0.370107f, -0.200194f, 0.907162f),
	float3(-0.287536f, -0.785107f, 0.548571f),
	float3(0.719866f, -0.092327f, 0.687945f),
	float3(-0.352856f, 0.611746f, 0.707997f),
	float3(-0.020887f, 0.344649f, 0.938499f),
	float3(-0.818855f, 0.125091f, 0.560204f),
	float3(0.647126f, 0.452989f, 0.613212f),
	float3(0.089614f, 0.827422f, 0.554384f),
	float3(-0.639705f, -0.283298f, 0.714507f),
	float3(0.618983f, -0.740359f, 0.262161f),
	float3(0.554837f, -0.491453f, 0.67129f),
	float3(-0.079202f, 0.529802f, 0.844415f),
	float3(0.128125f, -0.30475f, 0.943775f),
	float3(-0.631872f, 0.503359f, 0.589379f),
	float3(-0.357161f, 0.859488f, 0.365673f),
	float3(0.564701f, 0.156425f, 0.810336f),
	float3(-0.020271f, -0.383473f, 0.92333f),
	float3(0.047743f, -0.828224f, 0.55836f),
	float3(0.309765f, -0.611821f, 0.727819f),
	float3(-0.69558f, 0.050553f, 0.716668f),
	float3(0.13251f, 0.130565f, 0.982545f),
	float3(-0.776545f, -0.170768f, 0.606479f),
	float3(-0.255819f, -0.625534f, 0.737064f),
	float3(0.418516f, 0.474732f, 0.774257f),
	float3(-0.202337f, 0.139102f, 0.969387f),
	float3(0.363534f, -0.727339f, 0.582083f),
	float3(0.011389f, -0.519204f, 0.854574f),
	float3(-0.546336f, 0.262712f, 0.795298f),
	float3(-0.174712f, -0.052854f, 0.9832f),
	float3(0.771837f, 0.587744f, 0.242538f),
	float3(-0.828806f, -0.042163f, 0.557945f),
	float3(-0.167485f, -0.735594f, 0.656392f),
	float3(-0.575383f, 0.384694f, 0.721765f),
	float3(0.610375f, 0.54918f, 0.570827f),
	float3(-0.734435f, -0.65434f, 0.180123f),
	float3(-0.361025f, -0.514422f, 0.777837f),
	float3(-0.414274f, 0.087558f, 0.905931f),
	float3(-0.103217f, 0.93529f, 0.338495f),
	float3(-0.765781f, -0.36018f, 0.532776f),
	float3(0.428122f, -0.54917f, 0.717721f),
	float3(0.506694f, -0.159087f, 0.847321f),
	float3(0.87715f, -0.447626f, 0.173893f),
	float3(-0.138816f, -0.870934f, 0.471385f),
	float3(-0.125746f, -0.522739f, 0.843168f),
	float3(0.262786f, 0.584786f, 0.767443f),
	float3(-0.879572f, 0.322952f, 0.349364f),
	float3(-0.473778f, 0.621974f, 0.623444f),
	float3(0.309945f, -0.337078f, 0.888995f),
	float3(-0.163784f, 0.270479f, 0.948692f),
	float3(0.8198f, -0.569273f, 0.0620984f),
	float3(0.459482f, -0.880552f, 0.116209f),
	float3(-0.542116f, -0.24236f, 0.804594f),
	float3(0.186253f, 0.280349f, 0.941655f),
	float3(0.185817f, 0.819586f, 0.541988f),
	float3(-0.338316f, 0.727947f, 0.596352f),
	float3(0.689812f, 0.037272f, 0.723028f),
	float3(-0.232089f, -0.179794f, 0.955934f),
	float3(0.79989f, 0.194191f, 0.567861f),
	float3(0.464642f, 0.623217f, 0.629054f),
	float3(-0.541547f, 0.044018f, 0.839517f),
	float3(0.205901f, -0.198658f, 0.958196f),
	float3(-0.457855f, 0.735466f, 0.499458f),
	float3(-0.077566f, 0.638211f, 0.765944f),
	float3(0.662408f, 0.270384f, 0.698647f),
	float3(0.274274f, 0.167354f, 0.946977f)
};

#pragma endregion

RayTracedAmbientOcclusion::RayTracedAmbientOcclusion()
	: m_sampleKernelSize(_countof(aoSamples))
	, m_frameNo(0)
	, m_sampleKernel(nullptr)
	, m_viewCB(nullptr)
	, m_rtaoCB(nullptr)
{

}

RayTracedAmbientOcclusion::~RayTracedAmbientOcclusion()
{
	delete m_sampleKernel;
	delete m_viewCB;
	delete m_rtaoCB;
}

void RayTracedAmbientOcclusion::Initialize(UINT width, UINT height)
{
	m_ao[eFrame::Even].Initialize(width, height, false, Renderer::RTFormat_RTAO);
	m_ao[eFrame::Odd].Initialize(width, height, false, Renderer::RTFormat_RTAO);
	m_tempAO.Initialize(width, height, false, Renderer::RTFormat_AO);
	m_finalAO.Initialize(width, height, false, Renderer::RTFormat_AO);

	if (m_sampleKernel == nullptr)
	{
		m_sampleKernel = new Graphics::GPUBuffer();

		GPUBufferDesc bd;
		bd.BindFlags = BIND_SHADER_RESOURCE;
		bd.Usage = USAGE_IMMUTABLE;
		bd.MiscFlags = RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(float3) * (UINT)m_sampleKernelSize;
		bd.StructureByteStride = sizeof(float3);
		SubresourceData initData;
		initData.SysMem = aoSamples;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_sampleKernel);
		Renderer::GetDevice()->TransitionBarrier(m_sampleKernel, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	if (m_viewCB == nullptr)
	{
		m_viewCB = new Graphics::GPUBuffer();

		RTAOViewConstants viewCB;
		ZeroMemory(&viewCB, sizeof(viewCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(RTAOViewConstants);
		SubresourceData initData;
		initData.SysMem = &viewCB;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_viewCB);
		Renderer::GetDevice()->TransitionBarrier(m_viewCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (m_rtaoCB == nullptr)
	{
		m_rtaoCB = new Graphics::GPUBuffer();

		RTAOConstants aoCB;
		ZeroMemory(&aoCB, sizeof(aoCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(RTAOViewConstants);
		SubresourceData initData;
		initData.SysMem = &aoCB;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_rtaoCB);
		Renderer::GetDevice()->TransitionBarrier(m_rtaoCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	if (m_lowPassFilterCB == nullptr)
	{
		m_lowPassFilterCB = new Graphics::GPUBuffer();

		RTAOConstants aoCB;
		ZeroMemory(&aoCB, sizeof(aoCB));

		GPUBufferDesc bd;
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.Usage = USAGE_DEFAULT;
		bd.CpuAccessFlags = 0;
		bd.ByteWidth = sizeof(LowPassFilterConstants);
		SubresourceData initData;
		initData.SysMem = &aoCB;
		Renderer::GetDevice()->CreateBuffer(bd, &initData, m_lowPassFilterCB);
		Renderer::GetDevice()->TransitionBarrier(m_lowPassFilterCB, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
}

void RayTracedAmbientOcclusion::UpdateConstants(Renderer* renderer, const Camera& camera)
{
	m_frameNo = renderer->GetDevice()->GetCurrentFrameIndex();

	{
		RTAOViewConstants viewCB;
		viewCB.View = camera.m_view;
		viewCB.PrevView = camera.m_prevView;
		viewCB.PrevViewProj = camera.m_prevViewProj;
		viewCB.EyePos = float4(camera.m_eyePos.x, camera.m_eyePos.y, camera.m_eyePos.z, 0);
		float2 screenSize = float2((float)m_ao[0].GetDesc().Width, (float)m_ao[0].GetDesc().Height);
		float aspectRatio = screenSize.x / screenSize.y;
		viewCB.ResolutionTanHalfFovYAndAspectRatio = float4(screenSize.x, screenSize.y, tanf(0.5f * camera.m_fov), aspectRatio);
		viewCB.CameraNearFar = float2(camera.m_nearZ, camera.m_farZ);
		renderer->GetDevice()->UpdateBuffer(m_viewCB, &viewCB, sizeof(RTAOViewConstants));
	}

	{
		RTAOConstants rtaoCB;
		rtaoCB.AORadius = UIContext::OcclusionRadius;
		rtaoCB.AOStrength = UIContext::OcclusionPower;
		rtaoCB.FrameNo = m_frameNo;
		rtaoCB.SampleCount = UIContext::OcclusionSamplesCount;
		switch (rtaoCB.SampleCount)
		{
			case 1: rtaoCB.SampleStartIndex = 0; break;
			case 2: rtaoCB.SampleStartIndex = 36; break;
			case 3: rtaoCB.SampleStartIndex = 36 + 72; break;
			case 4: rtaoCB.SampleStartIndex = 36 + 72 + 108; break;
		}
		renderer->GetDevice()->UpdateBuffer(m_rtaoCB, &rtaoCB, sizeof(RTAOConstants));
	}

	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::RGS, m_viewCB, 0, RT_PASS_AMBIENT_OCCLUSION);
	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::RGS, m_rtaoCB, 1, RT_PASS_AMBIENT_OCCLUSION);
}

void RayTracedAmbientOcclusion::ComputeAO(Renderer* renderer)
{
	bool isEvenFrame = (renderer->GetDevice()->GetCurrentFrameIndex() % 2) == 0;

	Texture2D* outAO = m_ao[isEvenFrame ? eFrame::Even : eFrame::Odd].GetTexture(0);
	Texture2D* prevAO = m_ao[isEvenFrame ? eFrame::Odd : eFrame::Even].GetTexture(0);

	renderer->GetDevice()->BindUnorderedAccessResource(RGS, outAO, 0, -1, RT_PASS_AMBIENT_OCCLUSION);
	renderer->GetDevice()->BindResource(RGS, m_sampleKernel, 4, -1, RT_PASS_AMBIENT_OCCLUSION);
	renderer->GetDevice()->BindResource(RGS, prevAO, 5, -1, RT_PASS_AMBIENT_OCCLUSION);

	RayTracePSO* rtpso = renderer->GetPSO(eRTPSO::RTAO);
	renderer->GetDevice()->BindRayTracePSO(rtpso);

	ShaderTable* stb = renderer->GetSTB(eRTPSO::RTAO);

	UINT w = outAO->m_desc.Width;
	UINT h = outAO->m_desc.Height;

	DispatchRaysDesc dispatchRaysDesc = renderer->InitializeDispatchRaysDesc(rtpso, stb, w, h, 1, RT_PASS_AMBIENT_OCCLUSION);

	renderer->GetDevice()->DispatchRays(dispatchRaysDesc);
}

void RayTracedAmbientOcclusion::LowPassFilter(Renderer* renderer, const Camera& camera)
{
	bool isEvenFrame = (renderer->GetDevice()->GetCurrentFrameIndex() % 2) == 0;

	LowPassFilterConstants filterCB;
	float2 screenSize = float2((float)m_ao[0].GetDesc().Width, (float)m_ao[0].GetDesc().Height);
	filterCB.InvView = camera.m_invView;
	filterCB.TexelSize = screenSize;

	renderer->GetDevice()->BindSampler(PS, renderer->GetSamplerState(eSamplerState::LinearClamp), 0);
	renderer->GetDevice()->BindGraphicsPSO(renderer->GetPSO(eGPSO::LowPassFilter));
	renderer->GetDevice()->BindConstantBuffer(SHADERSTAGE::PS, m_lowPassFilterCB, 0);

	{
		filterCB.FilterDirection = float2(1.0f, 0.0f);
		filterCB.DoTemporalFilter = 1;
		renderer->GetDevice()->UpdateBuffer(m_lowPassFilterCB, &filterCB, sizeof(LowPassFilterConstants));
	}
	
	{
		m_tempAO.Activate();
		Texture2D* aoTex = m_ao[isEvenFrame ? eFrame::Even : eFrame::Odd].GetTexture(0);
		renderer->GetDevice()->BindResource(PS, aoTex, 2);
		renderer->GetDevice()->DrawInstanced(3, 1, 0, 0);
		m_tempAO.Deactivate();
	}

	{
		filterCB.FilterDirection = float2(0.0f, 1.0f);
		filterCB.DoTemporalFilter = 0;
		renderer->GetDevice()->UpdateBuffer(m_lowPassFilterCB, &filterCB, sizeof(LowPassFilterConstants));
	}

	{
		m_finalAO.Activate();
		renderer->GetDevice()->BindResource(PS, m_tempAO.GetTexture(0), 3);
		renderer->GetDevice()->DrawInstanced(3, 1, 0, 0);
		m_finalAO.Deactivate();
	}
}
