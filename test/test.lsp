(comment "CHANGELOG
          2024/09/28: Created test file and added code to it.
          2024/09/29: Added code to it.
          2024/09/30: Added changelog.
          2024/10/02: Better syntax.
          2024/10/03: Better syntax.
          2024/10/09: Added a test.")

(strdef name (input "Name > "))

(fncdef hello (params name)
    (print (+ (+ "Hello world " name) "!"))
)

(hello "someone")
(hello name)
