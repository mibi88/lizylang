(comment "CHANGELOG
          2024/10/08: Created this file.")

(strdef str1 "A string")
(strdef str2 "Another string")
(strdef list1 (++ str1 str2))

(print (++ "A string" "Another string"))
(print (++ str1 str2))
(print list1)

(numdef num1 3.5)
(numdef num2 5.1)
(numdef list2 (++ num1 num2))

(print (++ 3.5 5.1))
(print (++ num1 num2))
(print list2)

(print (list "A" "list" "of" "strings"))

(print (list 2 3.5 1 3))

(print (list))

(numdef voidlist (list))
(print voidlist)
