(comment "CHANGELOG
          2024/10/04: Created this test file.
          2024/10/07: Call print.
          2024/10/09: Add test.")

(strdef str "Hello!")
(print str)

(strdef str2 str)
(print str2)

(comment "The two following call should fail: the variable already exists.")
(strdef str "Something else")
