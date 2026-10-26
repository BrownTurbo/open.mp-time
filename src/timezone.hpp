#pragma once
#ifndef CHRONO_TIMEZONE_H
#define CHRONO_TIMEZONE_H

#include <string>
#include <ctime>
#include <unordered_map>
#include <algorithm>

enum class DstRule {
    None,
    USA,       // 2nd Sunday in March to 1st Sunday in Nov
    Europe,    // Last Sunday in March to Last Sunday in Oct
    Australia  // 1st Sunday in Oct to 1st Sunday in April
};

struct TimezoneInfo {
    int standardOffsetSeconds;
    DstRule dstRule;
};

inline bool isDstActive(DstRule rule, std::time_t utcTime, int standardOffset) {
    if (rule == DstRule::None) return false;

    std::time_t localTime = utcTime + standardOffset;
    struct tm tmLocal;
#ifdef _WIN32
    gmtime_s(&tmLocal, &localTime);
#else
    gmtime_r(&localTime, &tmLocal);
#endif

    int year = tmLocal.tm_year + 1900;
    int month = tmLocal.tm_mon + 1;
    int day = tmLocal.tm_mday;
    int hour = tmLocal.tm_hour;

    auto dayOfWeek = [](int y, int m, int d) {
        static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        if (m < 3) y -= 1;
        return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
    };

    if (rule == DstRule::USA) {
        int dowMarch1 = dayOfWeek(year, 3, 1);
        int dstStartDay = 14 - ((dowMarch1 + 6) % 7);

        int dowNov1 = dayOfWeek(year, 11, 1);
        int dstEndDay = 7 - ((dowNov1 + 6) % 7);

        if (month < 3 || month > 11) return false;
        if (month > 3 && month < 11) return true;
        if (month == 3) {
            if (day < dstStartDay) return false;
            if (day > dstStartDay) return true;
            return hour >= 2;
        }
        if (month == 11) {
            if (day < dstEndDay) return true;
            if (day > dstEndDay) return false;
            return hour < 2;
        }
    } else if (rule == DstRule::Europe) {
        int dowMarch31 = dayOfWeek(year, 3, 31);
        int dstStartDay = 31 - dowMarch31;

        int dowOct31 = dayOfWeek(year, 10, 31);
        int dstEndDay = 31 - dowOct31;

        if (month < 3 || month > 10) return false;
        if (month > 3 && month < 10) return true;
        if (month == 3) {
            if (day < dstStartDay) return false;
            if (day > dstStartDay) return true;
            return hour >= 2;
        }
        if (month == 10) {
            if (day < dstEndDay) return true;
            if (day > dstEndDay) return false;
            return hour < 2;
        }
    } else if (rule == DstRule::Australia) {
        int dowOct1 = dayOfWeek(year, 10, 1);
        int dstStartDay = 7 - ((dowOct1 + 6) % 7);

        int dowApril1 = dayOfWeek(year, 4, 1);
        int dstEndDay = 7 - ((dowApril1 + 6) % 7);

        if (month > 4 && month < 10) return false;
        if (month < 4 || month > 10) return true;
        if (month == 10) {
            if (day < dstStartDay) return false;
            if (day > dstStartDay) return true;
            return hour >= 2;
        }
        if (month == 4) {
            if (day < dstEndDay) return true;
            if (day > dstEndDay) return false;
            return hour < 3;
        }
    }

    return false;
}

inline const std::unordered_map<std::string, TimezoneInfo>& GetTimezoneMap() {
    static const std::unordered_map<std::string, TimezoneInfo> tzMap = {
        // Common abbreviations
        { "utc", { 0, DstRule::None } },
        { "gmt", { 0, DstRule::None } },
        { "est", { -18000, DstRule::USA } },
        { "edt", { -14400, DstRule::None } },
        { "cst", { -21600, DstRule::USA } },
        { "cdt", { -18000, DstRule::None } },
        { "mst", { -25200, DstRule::USA } },
        { "mdt", { -21600, DstRule::None } },
        { "pst", { -28800, DstRule::USA } },
        { "pdt", { -25200, DstRule::None } },
        { "cet", { 3600, DstRule::Europe } },
        { "cest", { 7200, DstRule::None } },
        { "eet", { 7200, DstRule::Europe } },
        { "eest", { 10800, DstRule::None } },
        { "jst", { 32400, DstRule::None } },
        { "ist", { 19800, DstRule::None } },
// Auto-generated from time.inc
        { "africa/abidjan", { 0, DstRule::None } },
        { "africa/accra", { 0, DstRule::None } },
        { "africa/addis_ababa", { 10800, DstRule::None } },
        { "africa/algiers", { 3600, DstRule::None } },
        { "africa/asmara", { 10800, DstRule::None } },
        { "africa/bamako", { 0, DstRule::None } },
        { "africa/bangui", { 3600, DstRule::None } },
        { "africa/banjul", { 0, DstRule::None } },
        { "africa/bissau", { 0, DstRule::None } },
        { "africa/blantyre", { 7200, DstRule::None } },
        { "africa/brazzaville", { 3600, DstRule::None } },
        { "africa/bujumbura", { 7200, DstRule::None } },
        { "africa/cairo", { 7200, DstRule::None } },
        { "africa/casablanca", { 0, DstRule::None } },
        { "africa/ceuta", { 7200, DstRule::None } },
        { "africa/conakry", { 0, DstRule::None } },
        { "africa/dakar", { 0, DstRule::None } },
        { "africa/dar_es_salaam", { 10800, DstRule::None } },
        { "africa/djibouti", { 10800, DstRule::None } },
        { "africa/douala", { 3600, DstRule::None } },
        { "africa/el_aaiun", { 0, DstRule::None } },
        { "africa/freetown", { 0, DstRule::None } },
        { "africa/gaborone", { 7200, DstRule::None } },
        { "africa/harare", { 7200, DstRule::None } },
        { "africa/johannesburg", { 7200, DstRule::None } },
        { "africa/juba", { 10800, DstRule::None } },
        { "africa/kampala", { 10800, DstRule::None } },
        { "africa/khartoum", { 10800, DstRule::None } },
        { "africa/kigali", { 7200, DstRule::None } },
        { "africa/kinshasa", { 3600, DstRule::None } },
        { "africa/lagos", { 3600, DstRule::None } },
        { "africa/libreville", { 3600, DstRule::None } },
        { "africa/lome", { 0, DstRule::None } },
        { "africa/luanda", { 3600, DstRule::None } },
        { "africa/lubumbashi", { 7200, DstRule::None } },
        { "africa/lusaka", { 7200, DstRule::None } },
        { "africa/malabo", { 3600, DstRule::None } },
        { "africa/maputo", { 7200, DstRule::None } },
        { "africa/maseru", { 7200, DstRule::None } },
        { "africa/mbabane", { 7200, DstRule::None } },
        { "africa/mogadishu", { 10800, DstRule::None } },
        { "africa/monrovia", { 0, DstRule::None } },
        { "africa/nairobi", { 10800, DstRule::None } },
        { "africa/ndjamena", { 3600, DstRule::None } },
        { "africa/niamey", { 3600, DstRule::None } },
        { "africa/nouakchott", { 0, DstRule::None } },
        { "africa/ouagadougou", { 0, DstRule::None } },
        { "africa/porto-novo", { 3600, DstRule::None } },
        { "africa/sao_tome", { 0, DstRule::None } },
        { "africa/tripoli", { 7200, DstRule::None } },
        { "africa/tunis", { 3600, DstRule::None } },
        { "africa/windhoek", { 3600, DstRule::None } },
        { "america/adak", { -32400, DstRule::USA } },
        { "america/anchorage", { -28800, DstRule::USA } },
        { "america/anguilla", { -14400, DstRule::None } },
        { "america/antigua", { -14400, DstRule::None } },
        { "america/araguaina", { -10800, DstRule::None } },
        { "america/argentina/buenos_aires", { -10800, DstRule::None } },
        { "america/argentina/catamarca", { -10800, DstRule::None } },
        { "america/argentina/cordoba", { -10800, DstRule::None } },
        { "america/argentina/jujuy", { -10800, DstRule::None } },
        { "america/argentina/la_rioja", { -10800, DstRule::None } },
        { "america/argentina/mendoza", { -10800, DstRule::None } },
        { "america/argentina/rio_gallegos", { -10800, DstRule::None } },
        { "america/argentina/salta", { -10800, DstRule::None } },
        { "america/argentina/san_juan", { -10800, DstRule::None } },
        { "america/argentina/san_luis", { -14400, DstRule::None } },
        { "america/argentina/tucuman", { -10800, DstRule::None } },
        { "america/argentina/ushuaia", { -10800, DstRule::None } },
        { "america/aruba", { -14400, DstRule::None } },
        { "america/asuncion", { -14400, DstRule::None } },
        { "america/atikokan", { -18000, DstRule::None } },
        { "america/bahia", { -10800, DstRule::None } },
        { "america/bahia_banderas", { -21600, DstRule::None } },
        { "america/barbados", { -14400, DstRule::None } },
        { "america/belem", { -10800, DstRule::None } },
        { "america/belize", { -21600, DstRule::None } },
        { "america/blanc-sablon", { -14400, DstRule::None } },
        { "america/boa_vista", { -14400, DstRule::None } },
        { "america/bogota", { -18000, DstRule::None } },
        { "america/boise", { -21600, DstRule::USA } },
        { "america/cambridge_bay", { -21600, DstRule::None } },
        { "america/campo_grande", { -14400, DstRule::None } },
        { "america/cancun", { -18000, DstRule::None } },
        { "america/caracas", { -16200, DstRule::None } },
        { "america/cayenne", { -10800, DstRule::None } },
        { "america/cayman", { -18000, DstRule::None } },
        { "america/chicago", { -18000, DstRule::USA } },
        { "america/chihuahua", { -21600, DstRule::None } },
        { "america/costa_rica", { -21600, DstRule::None } },
        { "america/creston", { -25200, DstRule::None } },
        { "america/cuiaba", { -14400, DstRule::None } },
        { "america/curacao", { -14400, DstRule::None } },
        { "america/danmarkshavn", { 0, DstRule::None } },
        { "america/dawson", { -25200, DstRule::None } },
        { "america/dawson_creek", { -25200, DstRule::None } },
        { "america/denver", { -21600, DstRule::USA } },
        { "america/detroit", { -14400, DstRule::USA } },
        { "america/dominica", { -14400, DstRule::None } },
        { "america/edmonton", { -21600, DstRule::USA } },
        { "america/eirunepe", { -14400, DstRule::None } },
        { "america/el_salvador", { -21600, DstRule::None } },
        { "america/fort_nelson", { -25200, DstRule::None } },
        { "america/fortaleza", { -10800, DstRule::None } },
        { "america/glace_bay", { -10800, DstRule::USA } },
        { "america/godthab", { -7200, DstRule::None } },
        { "america/goose_bay", { -10800, DstRule::USA } },
        { "america/grand_turk", { -14400, DstRule::None } },
        { "america/grenada", { -14400, DstRule::None } },
        { "america/guadeloupe", { -14400, DstRule::None } },
        { "america/guatemala", { -21600, DstRule::None } },
        { "america/guayaquil", { -18000, DstRule::None } },
        { "america/guyana", { -14400, DstRule::None } },
        { "america/halifax", { -10800, DstRule::USA } },
        { "america/havana", { -14400, DstRule::None } },
        { "america/hermosillo", { -25200, DstRule::None } },
        { "america/indiana/indianapolis", { -14400, DstRule::USA } },
        { "america/indiana/knox", { -18000, DstRule::USA } },
        { "america/indiana/marengo", { -14400, DstRule::USA } },
        { "america/indiana/petersburg", { -14400, DstRule::USA } },
        { "america/indiana/tell_city", { -18000, DstRule::USA } },
        { "america/indiana/vevay", { -14400, DstRule::USA } },
        { "america/indiana/vincennes", { -14400, DstRule::USA } },
        { "america/indiana/winamac", { -14400, DstRule::USA } },
        { "america/inuvik", { -21600, DstRule::None } },
        { "america/iqaluit", { -14400, DstRule::USA } },
        { "america/jamaica", { -18000, DstRule::None } },
        { "america/juneau", { -28800, DstRule::USA } },
        { "america/kentucky/louisville", { -14400, DstRule::USA } },
        { "america/kentucky/monticello", { -14400, DstRule::USA } },
        { "america/kralendijk", { -14400, DstRule::None } },
        { "america/la_paz", { -14400, DstRule::None } },
        { "america/lima", { -18000, DstRule::None } },
        { "america/los_angeles", { -25200, DstRule::USA } },
        { "america/lower_princes", { -14400, DstRule::None } },
        { "america/maceio", { -10800, DstRule::None } },
        { "america/managua", { -21600, DstRule::None } },
        { "america/manaus", { -14400, DstRule::None } },
        { "america/marigot", { -14400, DstRule::None } },
        { "america/martinique", { -14400, DstRule::None } },
        { "america/matamoros", { -18000, DstRule::None } },
        { "america/mazatlan", { -21600, DstRule::None } },
        { "america/menominee", { -18000, DstRule::USA } },
        { "america/merida", { -18000, DstRule::None } },
        { "america/metlakatla", { -28800, DstRule::USA } },
        { "america/mexico_city", { -18000, DstRule::None } },
        { "america/miquelon", { -7200, DstRule::None } },
        { "america/moncton", { -10800, DstRule::USA } },
        { "america/monterrey", { -18000, DstRule::None } },
        { "america/montevideo", { -10800, DstRule::None } },
        { "america/montserrat", { -14400, DstRule::None } },
        { "america/nassau", { -14400, DstRule::None } },
        { "america/new_york", { -14400, DstRule::USA } },
        { "america/nipigon", { -14400, DstRule::None } },
        { "america/nome", { -28800, DstRule::USA } },
        { "america/noronha", { -7200, DstRule::None } },
        { "america/north_dakota/beulah", { -21600, DstRule::USA } },
        { "america/north_dakota/center", { -18000, DstRule::USA } },
        { "america/north_dakota/new_salem", { -18000, DstRule::USA } },
        { "america/ojinaga", { -21600, DstRule::None } },
        { "america/panama", { -18000, DstRule::None } },
        { "america/pangnirtung", { -14400, DstRule::None } },
        { "america/paramaribo", { -10800, DstRule::None } },
        { "america/phoenix", { -25200, DstRule::None } },
        { "america/port-au-prince", { -18000, DstRule::None } },
        { "america/port_of_spain", { -14400, DstRule::None } },
        { "america/porto_velho", { -14400, DstRule::None } },
        { "america/puerto_rico", { -14400, DstRule::None } },
        { "america/punta_arenas", { -14400, DstRule::None } },
        { "america/rainy_river", { -18000, DstRule::None } },
        { "america/rankin_inlet", { -18000, DstRule::None } },
        { "america/recife", { -10800, DstRule::None } },
        { "america/regina", { -21600, DstRule::None } },
        { "america/resolute", { -18000, DstRule::None } },
        { "america/rio_branco", { -14400, DstRule::None } },
        { "america/santarem", { -10800, DstRule::None } },
        { "america/santiago", { -14400, DstRule::None } },
        { "america/santo_domingo", { -14400, DstRule::None } },
        { "america/sao_paulo", { -10800, DstRule::None } },
        { "america/scoresbysund", { 0, DstRule::None } },
        { "america/sitka", { -28800, DstRule::USA } },
        { "america/st_barthelemy", { -14400, DstRule::None } },
        { "america/st_johns", { -9000, DstRule::USA } },
        { "america/st_kitts", { -14400, DstRule::None } },
        { "america/st_lucia", { -14400, DstRule::None } },
        { "america/st_thomas", { -14400, DstRule::None } },
        { "america/st_vincent", { -14400, DstRule::None } },
        { "america/swift_current", { -21600, DstRule::None } },
        { "america/tegucigalpa", { -21600, DstRule::None } },
        { "america/thule", { -10800, DstRule::None } },
        { "america/thunder_bay", { -14400, DstRule::None } },
        { "america/tijuana", { -25200, DstRule::None } },
        { "america/toronto", { -14400, DstRule::USA } },
        { "america/tortola", { -14400, DstRule::None } },
        { "america/vancouver", { -25200, DstRule::USA } },
        { "america/whitehorse", { -25200, DstRule::None } },
        { "america/winnipeg", { -18000, DstRule::USA } },
        { "america/yakutat", { -28800, DstRule::USA } },
        { "america/yellowknife", { -21600, DstRule::None } },
        { "antarctica/casey", { 28800, DstRule::None } },
        { "antarctica/davis", { 25200, DstRule::None } },
        { "antarctica/dumontdurville", { 36000, DstRule::None } },
        { "antarctica/macquarie", { 36000, DstRule::None } },
        { "antarctica/mawson", { 21600, DstRule::None } },
        { "antarctica/mcmurdo", { 43200, DstRule::None } },
        { "antarctica/palmer", { -14400, DstRule::None } },
        { "antarctica/rothera", { -10800, DstRule::None } },
        { "antarctica/syowa", { 10800, DstRule::None } },
        { "antarctica/troll", { 7200, DstRule::None } },
        { "antarctica/vostok", { 21600, DstRule::None } },
        { "arctic/longyearbyen", { 7200, DstRule::None } },
        { "asia/aden", { 10800, DstRule::None } },
        { "asia/almaty", { 21600, DstRule::None } },
        { "asia/amman", { 10800, DstRule::None } },
        { "asia/anadyr", { -39600, DstRule::None } },
        { "asia/aqtau", { 18000, DstRule::None } },
        { "asia/aqtobe", { 18000, DstRule::None } },
        { "asia/ashgabat", { 18000, DstRule::None } },
        { "asia/atyrau", { 18000, DstRule::None } },
        { "asia/baghdad", { 10800, DstRule::None } },
        { "asia/bahrain", { 10800, DstRule::None } },
        { "asia/baku", { 18000, DstRule::None } },
        { "asia/bangkok", { 25200, DstRule::None } },
        { "asia/barnaul", { 25200, DstRule::None } },
        { "asia/beirut", { 10800, DstRule::None } },
        { "asia/bishkek", { 21600, DstRule::None } },
        { "asia/brunei", { 28800, DstRule::None } },
        { "asia/chita", { 36000, DstRule::None } },
        { "asia/choibalsan", { 28800, DstRule::None } },
        { "asia/colombo", { 19800, DstRule::None } },
        { "asia/damascus", { 10800, DstRule::None } },
        { "asia/dhaka", { 25200, DstRule::None } },
        { "asia/dili", { 32400, DstRule::None } },
        { "asia/dubai", { 14400, DstRule::None } },
        { "asia/dushanbe", { 18000, DstRule::None } },
        { "asia/famagusta", { 10800, DstRule::None } },
        { "asia/gaza", { 10800, DstRule::None } },
        { "asia/hebron", { 10800, DstRule::None } },
        { "asia/ho_chi_minh", { 25200, DstRule::None } },
        { "asia/hong_kong", { 28800, DstRule::None } },
        { "asia/hovd", { 25200, DstRule::None } },
        { "asia/irkutsk", { 32400, DstRule::None } },
        { "asia/jakarta", { 25200, DstRule::None } },
        { "asia/jayapura", { 32400, DstRule::None } },
        { "asia/jerusalem", { 10800, DstRule::None } },
        { "asia/kabul", { 16200, DstRule::None } },
        { "asia/kamchatka", { -39600, DstRule::None } },
        { "asia/karachi", { 21600, DstRule::None } },
        { "asia/kathmandu", { 20700, DstRule::None } },
        { "asia/khandyga", { 39600, DstRule::None } },
        { "asia/kolkata", { 19800, DstRule::None } },
        { "asia/krasnoyarsk", { 28800, DstRule::None } },
        { "asia/kuala_lumpur", { 28800, DstRule::None } },
        { "asia/kuching", { 28800, DstRule::None } },
        { "asia/kuwait", { 10800, DstRule::None } },
        { "asia/macau", { 28800, DstRule::None } },
        { "asia/magadan", { 43200, DstRule::None } },
        { "asia/makassar", { 28800, DstRule::None } },
        { "asia/manila", { 28800, DstRule::None } },
        { "asia/muscat", { 14400, DstRule::None } },
        { "asia/nicosia", { 10800, DstRule::None } },
        { "asia/novokuznetsk", { 28800, DstRule::None } },
        { "asia/novosibirsk", { 25200, DstRule::None } },
        { "asia/omsk", { 25200, DstRule::None } },
        { "asia/oral", { 18000, DstRule::None } },
        { "asia/phnom_penh", { 25200, DstRule::None } },
        { "asia/pontianak", { 25200, DstRule::None } },
        { "asia/pyongyang", { 32400, DstRule::None } },
        { "asia/qatar", { 10800, DstRule::None } },
        { "asia/qyzylorda", { 21600, DstRule::None } },
        { "asia/riyadh", { 10800, DstRule::None } },
        { "asia/sakhalin", { 39600, DstRule::None } },
        { "asia/samarkand", { 18000, DstRule::None } },
        { "asia/seoul", { 32400, DstRule::None } },
        { "asia/shanghai", { 28800, DstRule::None } },
        { "asia/singapore", { 28800, DstRule::None } },
        { "asia/srednekolymsk", { 43200, DstRule::None } },
        { "asia/taipei", { 28800, DstRule::None } },
        { "asia/tashkent", { 18000, DstRule::None } },
        { "asia/tbilisi", { 14400, DstRule::None } },
        { "asia/tehran", { 16200, DstRule::None } },
        { "asia/thimphu", { 21600, DstRule::None } },
        { "asia/tokyo", { 32400, DstRule::None } },
        { "asia/tomsk", { 25200, DstRule::None } },
        { "asia/ulaanbaatar", { 28800, DstRule::None } },
        { "asia/urumqi", { 21600, DstRule::None } },
        { "asia/ust-nera", { 43200, DstRule::None } },
        { "asia/vientiane", { 25200, DstRule::None } },
        { "asia/vladivostok", { 39600, DstRule::None } },
        { "asia/yakutsk", { 36000, DstRule::None } },
        { "asia/yangon", { 23400, DstRule::None } },
        { "asia/yekaterinburg", { 21600, DstRule::None } },
        { "asia/yerevan", { 18000, DstRule::None } },
        { "atlantic/azores", { 0, DstRule::None } },
        { "atlantic/bermuda", { -10800, DstRule::None } },
        { "atlantic/canary", { 3600, DstRule::None } },
        { "atlantic/cape_verde", { -3600, DstRule::None } },
        { "atlantic/faroe", { 3600, DstRule::None } },
        { "atlantic/madeira", { 3600, DstRule::None } },
        { "atlantic/reykjavik", { 0, DstRule::None } },
        { "atlantic/south_georgia", { -7200, DstRule::None } },
        { "atlantic/st_helena", { 0, DstRule::None } },
        { "atlantic/stanley", { -14400, DstRule::None } },
        { "australia/adelaide", { 34200, DstRule::Australia } },
        { "australia/brisbane", { 36000, DstRule::None } },
        { "australia/broken_hill", { 34200, DstRule::Australia } },
        { "australia/currie", { 36000, DstRule::Australia } },
        { "australia/darwin", { 34200, DstRule::None } },
        { "australia/eucla", { 31500, DstRule::None } },
        { "australia/hobart", { 36000, DstRule::Australia } },
        { "australia/lindeman", { 36000, DstRule::None } },
        { "australia/lord_howe", { 37800, DstRule::Australia } },
        { "australia/melbourne", { 36000, DstRule::Australia } },
        { "australia/perth", { 28800, DstRule::None } },
        { "australia/sydney", { 36000, DstRule::Australia } },
        { "canada/atlantic", { -10800, DstRule::USA } },
        { "canada/central", { -18000, DstRule::USA } },
        { "canada/eastern", { -14400, DstRule::USA } },
        { "canada/mountain", { -21600, DstRule::USA } },
        { "canada/newfoundland", { -9000, DstRule::USA } },
        { "canada/pacific", { -25200, DstRule::USA } },
        { "europe/amsterdam", { 7200, DstRule::Europe } },
        { "europe/andorra", { 7200, DstRule::Europe } },
        { "europe/astrakhan", { 14400, DstRule::None } },
        { "europe/athens", { 10800, DstRule::Europe } },
        { "europe/belgrade", { 7200, DstRule::Europe } },
        { "europe/berlin", { 7200, DstRule::Europe } },
        { "europe/bratislava", { 7200, DstRule::Europe } },
        { "europe/brussels", { 7200, DstRule::Europe } },
        { "europe/bucharest", { 10800, DstRule::Europe } },
        { "europe/budapest", { 7200, DstRule::Europe } },
        { "europe/busingen", { 7200, DstRule::Europe } },
        { "europe/chisinau", { 10800, DstRule::Europe } },
        { "europe/copenhagen", { 7200, DstRule::Europe } },
        { "europe/dublin", { 3600, DstRule::Europe } },
        { "europe/gibraltar", { 7200, DstRule::Europe } },
        { "europe/guernsey", { 3600, DstRule::Europe } },
        { "europe/helsinki", { 10800, DstRule::Europe } },
        { "europe/isle_of_man", { 3600, DstRule::Europe } },
        { "europe/istanbul", { 10800, DstRule::None } },
        { "europe/jersey", { 3600, DstRule::Europe } },
        { "europe/kaliningrad", { 10800, DstRule::Europe } },
        { "europe/kiev", { 10800, DstRule::Europe } },
        { "europe/kirov", { 14400, DstRule::Europe } },
        { "europe/lisbon", { 3600, DstRule::Europe } },
        { "europe/ljubljana", { 7200, DstRule::Europe } },
        { "europe/london", { 3600, DstRule::Europe } },
        { "europe/luxembourg", { 7200, DstRule::Europe } },
        { "europe/madrid", { 7200, DstRule::Europe } },
        { "europe/malta", { 7200, DstRule::Europe } },
        { "europe/mariehamn", { 10800, DstRule::Europe } },
        { "europe/minsk", { 10800, DstRule::None } },
        { "europe/monaco", { 7200, DstRule::Europe } },
        { "europe/moscow", { 14400, DstRule::None } },
        { "europe/oslo", { 7200, DstRule::Europe } },
        { "europe/paris", { 7200, DstRule::Europe } },
        { "europe/podgorica", { 7200, DstRule::Europe } },
        { "europe/prague", { 7200, DstRule::Europe } },
        { "europe/riga", { 10800, DstRule::Europe } },
        { "europe/rome", { 7200, DstRule::Europe } },
        { "europe/samara", { 18000, DstRule::None } },
        { "europe/san_marino", { 7200, DstRule::Europe } },
        { "europe/sarajevo", { 7200, DstRule::Europe } },
        { "europe/saratov", { 14400, DstRule::None } },
        { "europe/simferopol", { 10800, DstRule::Europe } },
        { "europe/skopje", { 7200, DstRule::Europe } },
        { "europe/sofia", { 10800, DstRule::Europe } },
        { "europe/stockholm", { 7200, DstRule::Europe } },
        { "europe/tallinn", { 10800, DstRule::Europe } },
        { "europe/tirane", { 7200, DstRule::Europe } },
        { "europe/ulyanovsk", { 14400, DstRule::None } },
        { "europe/uzhgorod", { 10800, DstRule::Europe } },
        { "europe/vaduz", { 7200, DstRule::Europe } },
        { "europe/vatican", { 7200, DstRule::Europe } },
        { "europe/vienna", { 7200, DstRule::Europe } },
        { "europe/vilnius", { 10800, DstRule::Europe } },
        { "europe/volgograd", { 14400, DstRule::Europe } },
        { "europe/warsaw", { 7200, DstRule::Europe } },
        { "europe/zagreb", { 7200, DstRule::Europe } },
        { "europe/zaporozhye", { 10800, DstRule::Europe } },
        { "europe/zurich", { 7200, DstRule::Europe } },
        { "gmt", { 0, DstRule::None } },
        { "indian/antananarivo", { 10800, DstRule::None } },
        { "indian/chagos", { 21600, DstRule::None } },
        { "indian/christmas", { 25200, DstRule::None } },
        { "indian/cocos", { 23400, DstRule::None } },
        { "indian/comoro", { 10800, DstRule::None } },
        { "indian/kerguelen", { 18000, DstRule::None } },
        { "indian/mahe", { 14400, DstRule::None } },
        { "indian/maldives", { 18000, DstRule::None } },
        { "indian/mauritius", { 14400, DstRule::None } },
        { "indian/mayotte", { 10800, DstRule::None } },
        { "indian/reunion", { 14400, DstRule::None } },
        { "pacific/apia", { -39600, DstRule::None } },
        { "pacific/auckland", { 43200, DstRule::None } },
        { "pacific/bougainville", { 36000, DstRule::None } },
        { "pacific/chatham", { -40500, DstRule::None } },
        { "pacific/chuuk", { 36000, DstRule::None } },
        { "pacific/easter", { -21600, DstRule::None } },
        { "pacific/efate", { 39600, DstRule::None } },
        { "pacific/enderbury", { -39600, DstRule::None } },
        { "pacific/fakaofo", { -39600, DstRule::None } },
        { "pacific/fiji", { 43200, DstRule::None } },
        { "pacific/funafuti", { 43200, DstRule::None } },
        { "pacific/galapagos", { -21600, DstRule::None } },
        { "pacific/gambier", { -32400, DstRule::None } },
        { "pacific/guadalcanal", { 39600, DstRule::None } },
        { "pacific/guam", { 36000, DstRule::None } },
        { "pacific/honolulu", { -36000, DstRule::None } },
        { "pacific/kiritimati", { -36000, DstRule::None } },
        { "pacific/kosrae", { 39600, DstRule::None } },
        { "pacific/kwajalein", { 43200, DstRule::None } },
        { "pacific/majuro", { 43200, DstRule::None } },
        { "pacific/marquesas", { -34200, DstRule::None } },
        { "pacific/midway", { -39600, DstRule::None } },
        { "pacific/nauru", { 43200, DstRule::None } },
        { "pacific/niue", { -39600, DstRule::None } },
        { "pacific/norfolk", { 41400, DstRule::None } },
        { "pacific/noumea", { 39600, DstRule::None } },
        { "pacific/pago_pago", { -39600, DstRule::None } },
        { "pacific/palau", { 32400, DstRule::None } },
        { "pacific/pitcairn", { -28800, DstRule::None } },
        { "pacific/pohnpei", { 39600, DstRule::None } },
        { "pacific/port_moresby", { 36000, DstRule::None } },
        { "pacific/rarotonga", { -36000, DstRule::None } },
        { "pacific/saipan", { 36000, DstRule::None } },
        { "pacific/tahiti", { -36000, DstRule::None } },
        { "pacific/tarawa", { 43200, DstRule::None } },
        { "pacific/tongatapu", { -39600, DstRule::None } },
        { "pacific/wake", { 43200, DstRule::None } },
        { "pacific/wallis", { 43200, DstRule::None } },
        { "us/alaska", { -28800, DstRule::USA } },
        { "us/arizona", { -25200, DstRule::None } },
        { "us/central", { -18000, DstRule::USA } },
        { "us/eastern", { -14400, DstRule::USA } },
        { "us/hawaii", { -36000, DstRule::None } },
        { "us/mountain", { -21600, DstRule::USA } },
        { "us/pacific", { -25200, DstRule::USA } },
        { "utc", { 0, DstRule::None } },


    };
    return tzMap;
}

inline int GetTimezoneOffsetSeconds(const std::string &tzInput, std::time_t t = 0) {
    if (t == 0) {
        t = std::time(nullptr);
    }

    std::string tz = tzInput;
    std::transform(tz.begin(), tz.end(), tz.begin(), [](unsigned char c) { return std::tolower(c); });

    if (tz.empty() || tz == "utc" || tz == "gmt" || tz == "z") {
        return 0;
    }

    if (tz == "local") {
        struct tm tmLocal;
#ifdef _WIN32
        localtime_s(&tmLocal, &t);
        long tzWest = 0;
        _get_timezone(&tzWest);
        return -tzWest + (tmLocal.tm_isdst > 0 ? 3600 : 0);
#else
        localtime_r(&t, &tmLocal);
        return static_cast<int>(tmLocal.tm_gmtoff);
#endif
    }

    const auto &tzMap = GetTimezoneMap();
    auto it = tzMap.find(tz);
    if (it != tzMap.end()) {
        const TimezoneInfo &info = it->second;
        int offset = info.standardOffsetSeconds;
        if (isDstActive(info.dstRule, t, offset)) {
            offset += 3600;
        }
        return offset;
    }

    return 0;
}

#endif // CHRONO_TIMEZONE_H
