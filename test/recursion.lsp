(comment "CHANGELOG
          2024/10:12: Created test file and added code to it.")

(fncdef count (params n))
    (print n)
    (callif (< n 10) count (+ n 1))
(defend)

(count 5)

(fncdef say_hello (params))
    (print "Hello!")
    (say_hello)
(defend)

(comment "A stack overflow error should happen.")
(say_hello)
