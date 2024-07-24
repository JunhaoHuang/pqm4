# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| raccoon-128 (3 executions) | m4 | AVG: 46,916,594 <br /> MIN: 36,404,005 <br /> MAX: 67,921,710 | AVG: 60,926,017 <br /> MIN: 60,887,395 <br /> MAX: 60,956,313 | AVG: 16,362,391 <br /> MIN: 16,323,974 <br /> MAX: 16,382,487 |
| raccoon-128 (3 executions) | ref | AVG: 46,018,003 <br /> MIN: 45,988,572 <br /> MAX: 46,054,226 | AVG: 85,219,344 <br /> MIN: 85,191,150 <br /> MAX: 85,256,221 | AVG: 21,820,157 <br /> MIN: 21,760,436 <br /> MAX: 21,872,297 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| raccoon-128 | m4 | 127,300 | 377,292 | 111,312 |
| raccoon-128 | ref | 164,252 | 377,292 | 111,960 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| raccoon-128 | m4 | 50.2% | 57.7% | 50.2% |
| raccoon-128 | ref | 0.0% | 0.0% | 0.0% |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| raccoon-128 | m4 | 14,956 | 0 | 0 | 14,956 |
| raccoon-128 | ref | 17,768 | 0 | 0 | 17,768 |
