#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

constexpr char kMongoDbUri[] = "mongodb://db:27017";
constexpr char kDatabaseName[] = "test_warrior_db";
constexpr char kCollectionName[] = "warrior_info";

const std::unordered_set<std::string> valid_skills = {
        "BJJ"
        , "Karate"
        , "Judo"
        , "KungFu"
        , "Capoeira"
        , "Boxing"
        , "Taekwondo"
        , "Aikido"
        , "KravMaga"
        , "MuayThai"
        , "KickBoxing"
        , "Pankration"
        , "Wrestling"
        , "Sambo"
        , "Savate"
        , "Sumo"
        , "Kendo"
        , "Hapkido"
        , "LutaLivre"
        , "WingChu"
        , "Ninjutsu"
        , "Fencing"
        , "ArmWrestling"
        , "SuckerPunch"
        , "44Magnum"
    };

#endif // CONSTANTS_HPP
