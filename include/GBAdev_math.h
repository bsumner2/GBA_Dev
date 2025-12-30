/** (C) 28 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_MATH_H_
#define _GBADEV_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */


#define Typedef_FixedU(basis_type, whole, fraction)  \
  typedef unsigned basis_type UFixed##whole##_##fraction##_t
#define Typedef_Fixed(basis_type, whole, fraction)  \
  typedef signed basis_type Fixed##whole##_##fraction##_t

#define Fixed_To_IntU(whole, fraction, value) ((u32)  \
  ((UFixed##whole##_##fraction##_t)(value)>>(fraction)))

#define Fixed_To_Int(whole, fraction, value) ((i32)  \
  ((Fixed##whole##_##fraction##_t)(value)>>(fraction)))

#define Int_To_FixedU(whole, fraction, value)  \
  ((UFixed##whole##_##fraction##_t)  \
   ((u32)(value)<<(fraction)))


#define Int_To_Fixed(whole, fraction, value)  \
  ((Fixed##whole##_##fraction##_t)  \
   ((i32)(value)<<(fraction)))

#define Fixed_Scale(fraction) (1 << (fraction))
#define Float_To_FixedU(whole, fraction, value)  \
  ((UFixed##whole##_##fraction##_t)  \
   ((value) * (float)Fixed_Scale(fraction)))

#define Float_To_Fixed(whole, fraction, value)  \
  ((Fixed##whole##_##fraction##_t)  \
   ((value) * (float)Fixed_Scale(fraction)))

#define Fixed_To_FloatU(whole, fraction, value)  \
  ((float)((UFixed##whole##_##fraction##_t)(value)) /  \
   (float)Fixed_Scale(fraction))

#define Fixed_To_Float(whole, fraction, value)  \
  ((float)((Fixed##whole##_##fraction##_t)(value)) /  \
   (float)Fixed_Scale(fraction))

#define Fixed_MultiplyU(whole, fraction, factor1, factor2)  \
  ((UFixed##whole##_##fraction##_t)  \
   (((factor1)*(factor2))>>(fraction)))
#define Fixed_Multiply(whole, fraction, factor1, factor2)  \
  ((Fixed##whole##_##fraction##_t)  \
   (((factor1)*(factor2))>>(fraction)))

#define Fixed_DivideU(whole, fraction, dividend, divisor)  \
  ((UFixed##whole##_##fraction##_t)  \
   (((dividend)*Fixed_Scale(fraction))/(divisor)))

#define Fixed_Divide(whole, fraction, dividend, divisor)  \
  ((Fixed##whole##_##fraction##_t)  \
   (((dividend)*Fixed_Scale(fraction))/(divisor)))

#define IDiv_Ceil(dividend, divisor) (((dividend)+((divisor)-1))/(divisor))

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_MATH_H_ */
