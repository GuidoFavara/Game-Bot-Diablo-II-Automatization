When i have free time i will organize this, it's just a mess for now, i have also to translate everything to english

TODO:

Subir .gif demostrando el funcionamiento. Done

Fotos y guia de configuración.

Guia de edición de tablas en mpq.

Debería buscar la forma de inyectarlo en la memoría a través de mi otro proyecto  de MapHack el cual ya se auto inyecta en la memoria on load

Traducir todo esto a ingles

# Game-Bot-Diablo-II-Automatization

<img align="center" alt="GIF" src="https://github.com/GuidoFavara/Game-Bot-Diablo-II-Automatization/blob/master/t_video5102979892828438749.gif?raw=true" width="500" height="320" />

La  idea: Un bot que haga un camino repetitivo, entre a un portal, se teletransporte 3 veces, ataque hasta que los bichos mueran
escanee los items que fueron dropeados y agarre SOLO los que  son de valor

La idea principal fue crear un script con el camino hasta el portal guardando cada mouse position, luego el attack y el pick it de items dropeados lo 
planee hacer con search pixel y que agarre los colores de items valiosos desde el MapHack del juego (para el cual cree un mod) podría ocultar los items que no me sirven para que el bot
sea mas rapido y eficiente, fue un gran error. Luego hacer un search y si no hay items deseados salir de la partida, crear otra y repetir en loop infinito

Pensé en dejar lo "facil" seleccionar crear enviar texto, crear partida y salir para el final ya que no variaría la posision del mouse para cada uno de estos  movimientos. gran error


Problemas encontrados:
*El camino hasta el portal parecía ser sencillo, así que empecé, marque la forma mas rápida medi los tiempos de sleep entre cada accion de movimiento lo repeti y estaba contento,
ahí fue cuando al crear varias partidas y repetir el proceso me dí cuenta que el path puede variar 6 veces con unos simples movimientos del terreno haciendo que entrar
al portal sea imposible (esto está hecho para evitar este tipo de bot), lo cual desconocía por qué eran centimetros a los cuales en el juego normalmente no se les presta atención) Tuve que:
encontrar esos 6 mapas darme cuenta donde estaba el error y agregarle mas mouse movements que no interfieran en el resto de partidas para que funcione siempre, la solución fue hacer mas clicks para posisionarme siempre en el mismo lugar antes del cambio del terreno, pero fue solucionado con mucha pasiencia y testeo.


*Ahora el problema es como el bot reconoce que el bicho murio para ello podría implementar pixeles en los bichos para que cuando el pixel desaparezca deje de atacar y empiece a scanear, pero dado que llevaría mucho tiempo y el bicho muere generalmente de 5 ataques o menos, lo ideal fue hacer que siempre ataque 5 veces y listo.
Ahora el dropeo items una vez terminado el attack debe ejecutar el pixel search para scanear y agarre lo valioso, lo más logico sería implementar un 
lector de texto pero es muy complejo y extenso de hacer para solamente un juego. Solución: implementar el pixel search, problema encontrado
los textos en el juego son muy chicos y el pixel reader selecciona la punta del item apenas aparece el color que quiero, solución modificar el texto
desde la database del juego (mod) pero que funcione online, solución: modificiar los caracteres (font) del juego para crear un elemento que no existe
que sería una barra del color que eliga para que el bot la busque (sería nuestro item). Aquí surge un problema (no tengo idea de como hacer todo esto,
fue solo una idea que parecía perfecta, tube que aprender todo)
Busque como modificar las fuentes que el juego utiliza, para ellos tuve que aprender como hacer un mod que funcione online y no singleplayer.
Extrayendo archivos .mpq (database) del juego, hay un archivo .dc6 dentro obtengo los  255 caracteres que es la font
de ahí identificar el  ] (a ese corchete le agergaría una sombra que una vez que inserte varios ]]]]] formarían una barra de un color solido ideal para el pixel search) una vez encontrado
ese archivo no sabía como modificarlo ya que son archivos únicos del juego no hay otros juegos que usen .dc6 por ende no hay programas para editarlos prácticamente, logré encontrar un 
archivo del 2000 llamado "dc6maker2", extraje el archivo pero ahora era una extensión .pcx otro problema mas, convertir pcx a jpg o lo que sea para editarlo con sombra, creando un batch
que cambie el formato lo logre hacer, editar y con otro batch que vuelva a unir todo dentro del .dc6 original funcinó pero antes de ésto debía modificar el juego para que *-*Guido MIra esto que pasa si agrego corchetes al maphack me pondra la barra?*-*

*Ahora tenía el camino, el ataque, la forma de encontrar los items, todo debía funcionar, pero no. Nuevo error el search pixel marca el primer pixel del color seleccionado y no era 
posible que agarre el item que deseo ya que el nombre es muy corto y para que el item sea levantado del suelo se necesita hacer click cerca del medio del nombre y no en la puntita (como hace 
el search pixel), ahora debía ver como solucionar esto, lo ideal era agrandar el espacio de fondo que lleva el texto (nombre del item) forma tal queden espacios vacios grandes y el nombre 
del item centrado para que cuando haga click lo levante.
Para solucionarlo había que ir a las tablas en patch_d2.mqp dentro hay un folder "data\local\LNG\ESP" y contiene expansionstrings.tbl, patchstrings.tbl y stings.tbl, con el editor
QTBL encontrar en la lista de todos los item del juego, el nombre de los que quiero que el bot pickee agregarle espacio antes y despues del texto y que queden como adjunto en la imagen

Que tiene de malo y bueno este bot?
No se inyecta en la memoria por lo cual no va a ser detectado por anti cheats del juego pero su punto contraproducente es que mientras este corriendo el bot no podré usar la pc
para ello la solución fue montar una VM y correr el bot desde ahí, el problema fue que el path y toda la configuración de ataque basada en movimientos de cursor preestablecidos
y hechos en base a la resolución de mi PC por lo cual escribí todo el código otra vez para la resolución de mi VM y ya quedó, también tuve que automatizar que siempre verifique la posicion
en donde la ventana es abierta, la acomode a la parte superior de esta forma ahora cualquier PC con mi resolucion de pantalla podría funcionar sin problemas no importa si la ventana es movida
el bot la acomoda, también incorporé un boton de pausa y continuación en caso de necesitar detener el bot se hace con SUPR (puede ser cambiado).

TODO: Interfaz gráfica, aunque al ser de uso personal no lo creo necesario, el tiempo lo dirá

