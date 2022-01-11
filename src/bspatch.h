extern "C" {
int bspatch(unsigned char *oldfile, ssize_t oldsize, const char *newfile, unsigned char *patch, ssize_t patchsize);
}
