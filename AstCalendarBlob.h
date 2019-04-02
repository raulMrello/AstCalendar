/*
 * AstCalendarBlob.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	AstCalendarBlob es el componente del módulo AstCalendar en el que se definen los objetos y tipos relativos a
 *	los objetos BLOB de este módulo.
 *	Todos los tipos definidos en este componente están asociados al namespace "Blob", de forma que puedan ser
 *	accesibles mediante el uso de: "Blob::"  e importando este archivo de cabecera.
 */
 
#ifndef __AstCalendarBlob__H
#define __AstCalendarBlob__H

#include "Blob.h"
#include "mbed.h"
  
#include "calendar_objects.h"



namespace Blob {


/** Tipo definido para referenciar la trama de arranque con la información del objeto calendar:manager */
typedef calendar_manager AstCalBootData_t;


}	// end namespace Blob



#endif
