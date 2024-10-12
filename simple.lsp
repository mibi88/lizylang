(comment "CHANGELOG
          2024/09/30: Created this test file.
          2024/10/04: Float test.
          2024/10/07: Print 3+2.5 and changed text.")

(strdef name (input "Name > "))

(print (+ (+ "Hello, " name) "!"))
(print (+ 3 2.5))
