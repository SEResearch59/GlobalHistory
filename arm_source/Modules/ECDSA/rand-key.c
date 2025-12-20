#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {
    unsigned long long secret;
    srand(time(NULL));
    // randomly generate secret and set
    secret = (1ULL * (rand() & 1) << 63) | (1ULL * rand() << 32) | (1ULL * (rand() & 1) << 31) | (1ULL * rand());
    printf("%llx\n", secret);
    return 0;
}
