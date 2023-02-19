def arr()
    var a[11], n = 11, sum in (
        for i = 0, i < n in (
            a[i] = i :
            sum = sum + a[i] * a[i]
        )
        end
    ) : sum
    end;