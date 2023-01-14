struct hamster_new_arg {
        char owner[32];
        char hamster[32];
        short treats;
};

struct hamster_lookup_arg {
    char owner[32];
    char hamster[32];
};

struct hamster_directory_arg {
    int fdptr;
    char owner[32];
    char hamster[32];
};

struct hamster_directory_ret {
    int res;
    int fdptr;
};

struct hamster_howsdoing_ret {
    int res;
    short treats_left;
    int rounds;
    short cost;
};

struct hamster_readentry_ret {
    int res;
    char owner[32];
    char hamster[32];
    short price;
};

struct hamster_givetreats_arg {
    int ID;
    short treats;
};

struct hamster_collect_arg {
    char owner[32];
};



program HAMSTER_RPC {
    version HAMSTER {
        int HAMSTER_NEW(hamster_new_arg) = 1;
        int HAMSTER_LOOKUP(hamster_lookup_arg) = 2;
        hamster_directory_ret HAMSTER_DIRECTORY(hamster_directory_arg) = 3;
        hamster_howsdoing_ret HAMSTER_HOWSDOING(int) = 4;
        hamster_readentry_ret HAMSTER_READENTRY(int) = 5;
        int HAMSTER_GIVETREATS(hamster_givetreats_arg) = 6;
        int HAMSTER_COLLECT(hamster_collect_arg) = 7;
    } = 1;
} = 0x2345121;