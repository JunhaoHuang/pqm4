# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| raccoon-128 (200 executions) | m4 | AVG: 25,380,780 <br /> MIN: 20,067,447 <br /> MAX: 30,729,039 | AVG: 46,366,816 <br /> MIN: 40,134,354 <br /> MAX: 52,637,965 | AVG: 13,419,478 <br /> MIN: 13,365,192 <br /> MAX: 13,492,433 |
| raccoon-128 (200 executions) | ref | AVG: 46,043,863 <br /> MIN: 45,874,539 <br /> MAX: 46,236,805 | AVG: 85,246,158 <br /> MIN: 85,077,530 <br /> MAX: 85,438,591 | AVG: 21,853,730 <br /> MIN: 21,712,770 <br /> MAX: 22,054,681 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
| kyber1024 | m4fspeed | 6,440 | 7,504 | 9,184 |
| kyber1024 | m4fstack | 3,336 | 3,376 | 4,952 |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| raccoon-128 | m4 | 164,252 | 377,292 | 111,312 |
| raccoon-128 | ref | 164,252 | 377,292 | 111,960 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| raccoon-128 | m4 | 63.8% | 70.0% | 60.2% |
| raccoon-128 | ref | 0.0% | 0.0% | 0.0% |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| raccoon-128 | m4 | 28,872 | 0 | 0 | 28,872 |
| raccoon-128 | ref | 17,768 | 0 | 0 | 17,768 |
