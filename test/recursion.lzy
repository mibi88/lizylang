(comment "CHANGELOG
          2024/10:12: Created test file and added code to it.
          2024/10/19: New function definition syntax.")

(fncdef count (params n)
    (print n)
    (callif (< n 10) count (+ n 1))
)

(count 5)

(fncdef say_hello (params)
    (print "Hello!")
    (say_hello)
)

(comment "A stack overflow error should happen.")
(say_hello)
