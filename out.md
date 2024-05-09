# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| dilithium3 (104 executions) | clean | AVG: 2,980,238 <br /> MIN: 2,979,134 <br /> MAX: 2,981,607 | AVG: 10,717,925 <br /> MIN: 4,610,607 <br /> MAX: 40,585,081 | AVG: 3,097,926 <br /> MIN: 3,097,615 <br /> MAX: 3,098,346 |
| dilithium3 (104 executions) | m4f | AVG: 2,395,161 <br /> MIN: 2,394,205 <br /> MAX: 2,396,206 | AVG: 5,564,511 <br /> MIN: 2,795,761 <br /> MAX: 15,698,138 | AVG: 2,290,616 <br /> MIN: 2,290,360 <br /> MAX: 2,291,025 |
| dilithium3 (104 executions) | m4plant | AVG: 2,395,302 <br /> MIN: 2,394,217 <br /> MAX: 2,396,457 | AVG: 5,512,466 <br /> MIN: 2,776,816 <br /> MAX: 19,142,633 | AVG: 2,290,618 <br /> MIN: 2,290,201 <br /> MAX: 2,290,910 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| dilithium3 | clean | 60,812 | 79,560 | 57,700 |
| dilithium3 | m4f | 60,836 | 68,976 | 57,832 |
| dilithium3 | m4plant | 60,836 | 68,868 | 57,832 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| dilithium3 | clean | 66.3% | 32.5% | 58.9% |
| dilithium3 | m4f | 82.3% | 60.7% | 79.6% |
| dilithium3 | m4plant | 82.3% | 60.3% | 79.6% |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| dilithium3 | clean | 7,372 | 0 | 0 | 7,372 |
| dilithium3 | m4f | 20,008 | 0 | 0 | 20,008 |
| dilithium3 | m4plant | 18,488 | 0 | 0 | 18,488 |
