def fib(n)
  var a, b = 1, next, i = 2 in (
    while i < n in (
      next = a + b :
        a = b :
        b = next :
        i = i + 1
    )
    end
  ) : next
  end;