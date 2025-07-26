VAR canGoLeft, canGoForward, canGoRight

while TRUE do
    if LOOK do
        break
    done

    left
    canGoLeft := TEST
    right

    canGoForward := TEST

    right
    canGoRight := TEST
    left

    if canGoLeft do
        left
        forward 1
        continue
    done

    if canGoForward do
        forward 1
        continue
    done

    if canGoRight do
        right
        forward 1
        continue
    done

    LEFT
    left
    forward 1
    continue
done
