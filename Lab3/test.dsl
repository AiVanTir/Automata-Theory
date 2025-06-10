VAR a, b, sumArr, hexVal, infVal, nanVal, flag, xorn, c

a := 10
b[0] := 5
b[1] := 15
sumArr := #b
hexVal := 0x1A
infVal := INF
nanVal := NAN

flag := TRUE
xorn := TRUE ^ FALSE

if a > b[0] do
    a := a - b[0]
elif a = b[0] do
    a := 0
else
    a := 100
done

function inc(x) do
    return x + 1
done

b[2] := inc(a)

look
test

forward 2
left
backward 1
right
load 5
drop 5

