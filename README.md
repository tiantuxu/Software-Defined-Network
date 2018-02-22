# Software-Defined-Network(SDN)

## Contributors
### Controller: Tiantu Xu
### Switch: Zhengzhe Zhu

# Usage
Sample usage can be seen in [test.sh](https://github.com/tiantuxu/Software-Defined-Network-SDN-/blob/master/test.sh)

## Lauch a controller
```
./controller.bin 5000 ../topology6.txt
```
## Launch a switch
```
./switch.bin switch-id localhost 5000
```

## Launch a switch with a failed link to a neighbor
```
./switch.bin switch-id localhost 5000 -f neighbor_id
```
