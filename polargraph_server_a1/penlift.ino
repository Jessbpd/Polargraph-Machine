/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Penlift.

Este es uno de los archivos principales del programa del servidor polargraph.
Este archivo contiene las llamadas de servo que suben o bajan el lápiz desde
la página.

El comportamiento del elevador de pluma es el siguiente:

Si se recibe un simple comando de "levantar el lápiz" o "levantar el lápiz" ("C14, FIN"), la máquina
No intente levantar el bolígrafo si cree que ya está levantado. Comprueba el valor de la
variable booleana global "isPenUp" para decidir esto.

Si se recibe un "bolígrafo arriba" calificado, que incluye una posición del bolígrafo (por ejemplo, "C14,150, END"),
luego se actualiza la variable de posición global "arriba", y el servo se mueve a esa posición,
incluso si ya está "activo". Porque, naturalmente, si la posición hacia arriba ha cambiado, incluso si
ya está activo, hay una buena posibilidad de que no sea suficiente.

Lo mismo ocurre con el
*/
#ifdef PENLIFT
void penlift_movePen(int start, int end, int delay_ms)
{
  penHeight.attach(PEN_HEIGHT_SERVO_PIN);
  if(start < end)
  {
    for (int i=start; i<=end; i++) 
    {
      penHeight.write(i);
      delay(delay_ms);
#ifdef DEBUG_PENLIFT
      Serial.println(i);
#endif
    }
  }
  else
  {
    for (int i=start; i>=end; i--) 
    {
      penHeight.write(i);
      delay(delay_ms);
#ifdef DEBUG_PENLIFT
      Serial.println(i);
#endif
    }
  }
  penHeight.detach();
}

void penlift_penUp()
{
  if (inNoOfParams > 1)
  {
#ifdef DEBUG_PENLIFT
    Serial.print("Penup with params");
#endif
    int positionToMoveFrom = isPenUp ? upPosition : downPosition;
    upPosition = atoi(inParam1);
    penlift_movePen(positionToMoveFrom, upPosition, penLiftSpeed);
  }
  else
  {
    if (isPenUp == false)
    {
      penlift_movePen(downPosition, upPosition, penLiftSpeed);
    }
  }
  isPenUp = true;
}

void penlift_penDown()
{
  // check to see if this is a multi-action command (if there's a
  // parameter then this sets the "down" motor position too).
  if (inNoOfParams > 1)
  {
    int positionToMoveFrom = isPenUp ? upPosition : downPosition;
    downPosition = atoi(inParam1);
    penlift_movePen(positionToMoveFrom, downPosition, penLiftSpeed);
  }
  else
  {
    if (isPenUp == true)
    {
      penlift_movePen(upPosition, downPosition, penLiftSpeed);
    }
  }
  isPenUp = false;
}
#endif
