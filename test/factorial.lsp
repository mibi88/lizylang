(comment "CHANGELOG
          2024/10/12: Created this file.
          2024/10/19: New function definition syntax.")

(fncdef factorial (params n)
    (* (callif (> n 1) factorial (- n 1)) n)
)

(print (factorial 6))
