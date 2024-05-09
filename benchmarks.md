# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| raccoon (1 executions) | m4 | AVG: 27,794,498 <br /> MIN: 27,794,498 <br /> MAX: 27,794,498 | AVG: 50,392,751 <br /> MIN: 50,392,751 <br /> MAX: 50,392,751 | AVG: 16,392,838 <br /> MIN: 16,392,838 <br /> MAX: 16,392,838 |
| raccoon (1 executions) | ref | AVG: 35,275,077 <br /> MIN: 35,275,077 <br /> MAX: 35,275,077 | AVG: 72,623,760 <br /> MIN: 72,623,760 <br /> MAX: 72,623,760 | AVG: 21,874,339 <br /> MIN: 21,874,339 <br /> MAX: 21,874,339 |
| raccoon-256 (1 executions) | ref | AVG: 73,798,596 <br /> MIN: 73,798,596 <br /> MAX: 73,798,596 | AVG: 123,914,134 <br /> MIN: 123,914,134 <br /> MAX: 123,914,134 | AVG: 60,759,467 <br /> MIN: 60,759,467 <br /> MAX: 60,759,467 |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| raccoon | m4 | 98,964 | 283,416 | 111,312 |
| raccoon | ref | 112,008 | 284,064 | 111,960 |
| raccoon-256 | ref | 140,704 | 505,320 | 185,832 |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| raccoon | m4 | 56.4% | 63.2% | 50.3% |
| raccoon | ref | 0.0% | 0.0% | 0.0% |
| raccoon-256 | ref | 0.0% | 0.0% | 0.0% |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| raccoon | m4 | 13,996 | 0 | 0 | 13,996 |
| raccoon | ref | 17,596 | 0 | 0 | 17,596 |
| raccoon-256 | ref | 16,596 | 0 | 0 | 16,596 |
