@*============================================================================*/
@* objects.mss: creates makefile dependacies for all game objects
@*============================================================================*/
#============================================================================*/
# object.ms: makefile dependancies for all oad based game objects
#   created from  object.mss,DO NOT MODIFY
#============================================================================*/

@define OBJECTSHEADER   @nOADOBJS= \
@define OBJECTSFOOTER
@define OBJECTENTRY(name,collide) $(OBJ_DIR)@lowercase(name)@+$(OBJ) \
@define OBJECTONLYTEMPLATEENTRY(name,collide) $(OBJ_DIR)@lowercase(name)@+$(OBJ) \
@define OBJECTNOACTORENTRY(name,collide) \
@define OBJECTSUBENTRY(name,parent,collide) \
@define COLTABLEHEADER
@define COLTABLEFOOTER
@define COLTABLEENTRY(obj1,obj2,msg1,msg2)

#============================================================================

@include objects.mac

#============================================================================
