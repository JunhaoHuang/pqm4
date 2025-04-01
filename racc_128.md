# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| raccoon-128 (10 executions) | m4 | AVG: 23,004,385 <br /> MIN: 22,970,949 <br /> MAX: 23,039,451 | AVG: 43,256,285 <br /> MIN: 43,224,378 <br /> MAX: 43,290,642 | AVG: 13,219,453 <br /> MIN: 13,193,341 <br /> MAX: 13,248,366 |
| raccoon-128 (10 executions) | ref | AVG: 35,260,549 <br /> MIN: 35,182,678 <br /> MAX: 35,364,076 | AVG: 72,610,435 <br /> MIN: 72,532,700 <br /> MAX: 72,713,059 | AVG: 21,855,538 <br /> MIN: 21,760,139 <br /> MAX: 21,986,330 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| raccoon-128 | m4 | 74,368 | 155,647 | 111,312 |
| raccoon-128 | ref | 111,900 | 284,064 | 111,960 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| raccoon-128 | m4 | 65.8% | 72.4% | 61.0% |
| raccoon-128 | ref | 0.0% | 0.0% | 0.0% |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| raccoon-128 | m4 | 30,408 | 0 | 0 | 30,408 |
| raccoon-128 | ref | 17,596 | 0 | 0 | 17,596 |
