(comment "CHANGELOG
          2024/10/09: Created this file.
          2024/10/12: Added print after call.
          2024/10/19: Test returned value. New function definition syntax.")

(fncdef say_hello (params)
    (print "Hello!")
)

(print (say_hello))
(print "Done :D!")
