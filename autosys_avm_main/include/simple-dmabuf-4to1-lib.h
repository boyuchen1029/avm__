
int run_avm(int argc, char **argv);

int exit_avm();

void view_4to1(void);

void view_front(void);

void view_back(void);

void view_left(void);

void view_right(void);

int get_layer_status(void);
int get_avm_status(void);

void layer_enabled(void);
void layer_disabled(void);

void SET_AVMPAGE_freeTouch(int page,int limit_theta[2], int limit_phi[2]);