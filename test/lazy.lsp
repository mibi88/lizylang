(comment "CHANGELOG
          2024/10/19: Created this file.")

(numdef a 25)
(numdef b 5)

(fncdef if2 (params a b c))
    (if a b c)
(defend)

(if2 1 (print (+ a b)) (print (- a b)))
(if2 0 (print (+ a b)) (print (- a b)))
