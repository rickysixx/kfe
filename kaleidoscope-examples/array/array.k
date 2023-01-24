def arr()
    var a[3], sum = 0 in (
        for i = 0, i < 3, 1.0 in
            a[i] = i * i :
            sum = sum + a[i]
        end
    ) : sum
    end;