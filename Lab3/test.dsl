VAR a, b, sumArr, hexVal, infVal, nanVal, flag, xorn, c, dist, obs
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
elseif a = b[0] do
    a := 0
done

b[2] := inc(a)

dist := see
obs  := see!

forward 2
left
backward 1
right
load 5
drop 5
