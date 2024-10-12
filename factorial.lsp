(comment "CHANGELOG
          2024/10/12: Created this file.")

(fncdef factorial (params n))
    (* (callif (> n 1) factorial (- n 1)) n)
(defend)

(print (factorial 6))
