//
// Fast arctan sourced from
// https://github.com/xiezhq-hermann/atan_lookup
//

#include <cmath>

#include "fast_arctan.hpp"

const double LUT_d[102] = {
        0,                   0.00999966668666524, 0.0199973339731505,  0.0299910048568779,  0.0399786871232900,
        0.0499583957219428,  0.0599281551212079,  0.0698860016346425,  0.0798299857122373,  0.0897581741899505,
        0.0996686524911620,  0.109559526773944,   0.119428926018338,   0.129275004048143,   0.139095941482071,
        0.148889947609497,   0.158655262186401,   0.168390157147530,   0.178092938231198,   0.187761946513593,
        0.197395559849881,   0.206992194219821,   0.216550304976089,   0.226068387993884,   0.235544980720863,
        0.244978663126864,   0.254368058553266,   0.263711834462266,   0.273008703086711,   0.282257421981491,
        0.291456794477867,   0.300605670042395,   0.309702944542456,   0.318747560420644,   0.327738506780556,
        0.336674819386727,   0.345555580581712,   0.354379919123438,   0.363147009946176,   0.371856073848581,
        0.380506377112365,   0.389097231055278,   0.397627991522129,   0.406098058317616,   0.414506874584786,
        0.422853926132941,   0.431138740718782,   0.439360887284591,   0.447519975157170,   0.455615653211225,
        0.463647609000806,   0.471615567862328,   0.479519291992596,   0.487358579505190,   0.495133263468404,
        0.502843210927861,   0.510488321916776,   0.518068528456721,   0.525583793551610,   0.533034110177490,
        0.540419500270584,   0.547740013715902,   0.554995727338587,   0.562186743900029,   0.569313191100662,
        0.576375220591184,   0.583373006993856,   0.590306746935372,   0.597176658092678,   0.603982978252998,
        0.610725964389209,   0.617405891751573,   0.624023052976757,   0.630577757214935,   0.637070329275684,
        0.643501108793284,   0.649870449411948,   0.656178717991395,   0.662426293833151,   0.668613567927821,
        0.674740942223553,   0.680808828915828,   0.686817649758645,   0.692767835397122,   0.698659824721463,
        0.704494064242218,   0.710271007486686,   0.715991114416300,   0.721654850864761,   0.727262687996690,
        0.732815101786507,   0.738312572517228,   0.743755584298860,   0.749144624606017,   0.754480183834406,
        0.759762754875771,   0.764992832710910,   0.770170914020331,   0.775297496812126,   0.780373080066636,
        0.785398163397448,   0.790373246728302
};

double fast_arctan(double x) {
    if (x >= 0) {
        if (x <= 1) {
            int index = static_cast<int>(std::round(x * 100));
            return (LUT_d[index] + (x * 100 - index) * (LUT_d[index + 1] - LUT_d[index]));
        } else {
            double re_x = 1 / x;
            int index = static_cast<int>(std::round(re_x * 100));
            return (M_PI_2 - (LUT_d[index] + (re_x * 100 - index) * (LUT_d[index + 1] - LUT_d[index])));
            // No recursive is better here
        }
    } else {
        if (x >= -1) {
            double abs_x = -x;
            int index = static_cast<int>(std::round(abs_x * 100));
            return -(LUT_d[index] + (abs_x * 100 - index) * (LUT_d[index + 1] - LUT_d[index]));
        } else {
            double re_x = 1 / (-x);
            int index = static_cast<int>(std::round(re_x * 100));
            return (LUT_d[index] + (re_x * 100 - index) * (LUT_d[index+1] - LUT_d[index])) - M_PI_2;
        }
    }
}
