# GlobalHistory
Open source PoC for global history side channel GHLeaker

Including Intel, AMD and ARM source code.

For each platform, the source code includes:
1. attack targeting if branch: run make if and run work-if.sh
2. attack targeting loop statement: run make loop and run work-loop.sh
3. attack targeting ECDSA: run test-ecdsa.sh
4. attack targeting RSA: run make rsa, insert module for RSA and run work-rsa.sh

The shift paremeter maybe different according to platforms. You can enumarte the shift parameter from 0 to the GHR size to migrate attack on your platform.
