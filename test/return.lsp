(comment "CHANGELOG
          2024/10/12: Created this file.")

(fncdef test (params a b)
    (+ (+ a (+ a b)) a)
)

(print (+ (print (+ "1" (test "1" "2"))) "3"))
