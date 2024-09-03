
default_name := "firmware"

build name=default_name:
    @zig build -Drelease \
    -Dname={{name}} \
    -Dld="./src/linker.ld" \
    -Dstartup="./src/startup.s" \
    -Dgpio \
    -Dinc="./src" \
    -Dsrc="./src/main.c" \
    -Dmacro="DEBUG;" \
    -Dinfo=true \
    -Dclangd=true \
    --summary all

size name=default_name:
    wc -c ./zig-out/bin/{{name}}.bin
    arm-none-eabi-size -A -x ./zig-out/bin/{{name}}
#    hexyl ./zig-out/bin/{{name}}.bin -g 4

dump name=default_name:
    arm-none-eabi-objdump \
    --source-comment \
    --disassemble \
    --architecture=armv8-m.base \
    -M force-thumb \
    ./zig-out/bin/{{name}} > ./zig-out/bin/{{name}}.s

flash name=default_name:
    pyocd list
    pyocd flash --target gd32f103c8 ./zig-out/bin/{{name}}.bin