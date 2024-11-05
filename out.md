# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| raccoon-128 (200 executions) | m4 | AVG: 76,629,617 <br /> MIN: 76,533,135 <br /> MAX: 76,778,076 | AVG: 130,525,042 <br /> MIN: 129,305,182 <br /> MAX: 354,074,328 | AVG: 13,224,336 <br /> MIN: 13,159,386 <br /> MAX: 13,330,477 |
| raccoon-128 (100 executions) | ref | AVG: 111,433,076 <br /> MIN: 111,290,079 <br /> MAX: 111,587,292 | AVG: 199,528,234 <br /> MIN: 199,391,312 <br /> MAX: 199,678,084 | AVG: 21,845,175 <br /> MIN: 21,718,129 <br /> MAX: 21,965,999 |
| raccoon-192 (100 executions) | m4 | AVG: 42,349,932 <br /> MIN: 42,251,560 <br /> MAX: 42,457,152 | AVG: 67,504,160 <br /> MIN: 67,410,748 <br /> MAX: 67,605,658 | AVG: 21,460,231 <br /> MIN: 21,388,696 <br /> MAX: 21,566,672 |
| raccoon-192 (100 executions) | ref | AVG: 68,028,007 <br /> MIN: 67,874,272 <br /> MAX: 68,257,878 | AVG: 108,952,874 <br /> MIN: 108,805,479 <br /> MAX: 109,166,603 | AVG: 35,868,812 <br /> MIN: 35,728,922 <br /> MAX: 36,023,721 |
| raccoon-256 (100 executions) | m4 | AVG: 54,053,044 <br /> MIN: 53,936,765 <br /> MAX: 54,185,864 | AVG: 85,786,656 <br /> MIN: 85,672,563 <br /> MAX: 85,914,779 | AVG: 36,094,837 <br /> MIN: 35,976,821 <br /> MAX: 36,230,703 |
| raccoon-256 (100 executions) | ref | AVG: 85,424,603 <br /> MIN: 85,214,220 <br /> MAX: 85,639,810 | AVG: 136,187,850 <br /> MIN: 135,994,166 <br /> MAX: 136,383,006 | AVG: 60,853,020 <br /> MIN: 60,652,226 <br /> MAX: 61,037,384 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| raccoon-128 | m4 | 262,736 | 557,604 | 111,312 |
| raccoon-128 | ref | 262,744 | 557,712 | 111,960 |
| raccoon-192 | m4 | 201,172 | 504,324 | 144,152 |
| raccoon-192 | ref | 201,180 | 274,431 | 144,800 |
| raccoon-256 | m4 | 181,120 | 582,560 | 185,184 |
| raccoon-256 | ref | 181,768 | 583,208 | 185,832 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
