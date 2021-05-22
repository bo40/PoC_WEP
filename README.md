# Proof of Concept for WEP attack using PTW method (Pychkine, Tews, Weinmann)

This tiny C program is implemented with reference to [this paper](https://eprint.iacr.org/2007/120)

# How to use
First, compile `generate-sample.c` and `attack_wep.c`.

## generate-sample.c
`generate-sample` generates a large number of the first 16 bytes of arp response encrypted by WEP, and write generated data to file.

The following generate file `data.out` which contains 100,000 pieces of data.
```
$ ./generate-sample 100000
```

## attack_wep.c
`attack_wep` attacks WEP and predict the root key.

The following attacks WEP by reading 80,000 pieces of data from the input file `data.out` and output the predicted root key.
```
$ ./attack_wep 80000 data.out
```