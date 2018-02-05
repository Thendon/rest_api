

//********************************************************
//README
//
//Die rest_connector.js erledigt eigentlich das Schlimmste
//automatisch, sowas wie Fehlerabfangen zum Beispiel.
//Also um die Rückgaben success & error rufen direkt den
//catch-Fall auf.
//In vars stehen in den Beispielen die nötigen (mit total
//dusseligen Werten gefüllten) Input-Variablen.
//
//REST.post( <path>, <inputs> ) ruft dann meinen server-shice
//auf und wenn alles klappt gehts in den then-Fall. Hier
//stehen in result alle Rückgaben drin die aber nur teilweise
//interessant für die Funktion sind (im Zweifelsfall einmal
//komplett printen).
//
//Wenn ein Fehler auftritt wird der catch-Fall aufgerufen.
//Im Parameter e steht hier auch als String
//meistens der korrekte Grund weshalb der Aufruf nicht
//funktioniert hat
//********************************************************
//UNWICHTIGER SCHEIß
//********************************************************

$( document ).ready(function() {
  $("#output").html( "ready!" );
});

function print(msg){
  console.log(msg);
  $("#output").html(msg);
}

//********************************************************
//HELFER VARIABLEN
//********************************************************

let SID = 0; //WICHTIG FÜR AUTHENTIFIZIERUNG

let myUsername; //Userdetails nur über username abrufbar

//********************************************************
//REGISTER FUNCTION
//********************************************************

function registerRandomUser(){
  var randomNumber = Math.floor( Math.random() * 10 );
  var vars = {
    username : "User" + randomNumber,
    pass : "pass" + randomNumber,
    firstname : "flow" + randomNumber,
    lastname : "herr" + randomNumber,
    birthdate : 123 + randomNumber,
    address : "Strasse 1" + randomNumber,
    city : "Osna",
    phone : "555 " + randomNumber + " nase",
  }

  REST.post( 'register', vars ).then( function( res ){
    print( "registerd user successfully!" );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//LOGIN FUNCTION
//********************************************************

function loginRandomUser(){
  var randomNumber = Math.floor( Math.random() * 10 );
  var vars = {
    username : "User" + randomNumber,
    init : true
  }

  REST.post( 'login', vars ).then( function( res ){
    print( "login process started, key = " + res.key );
    calcChecksum( randomNumber, res.key );
  }).catch(function( e ){
    print( e );
  });
}

//Vervollständigt den Login-Prozess
function calcChecksum( randomNumber, key ){
  var vars = {
    username : "User" + randomNumber,
    checksum : key * CRC32.str("pass" + randomNumber),
    init : false
  }

  REST.post( 'login', vars ).then( function( res ){
    SID = res.sessionID;
    //Um sicher zu gehen, dass der Username auch wirklich zur sessionID passt
    myUsername = res.username;
    print( "logged in as " + myUsername + ", sessionID = " + SID );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//LOGOUT FUNCTION
//********************************************************

function logoutLoggedIn(){
  var vars = {
    sessionID : SID
  }

  REST.post( 'logout', vars ).then( function( res ){
    print( "logged out " + SID );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//USER DETAILS
//********************************************************

function detailsUser1(){
  var vars = {
    sessionID : SID,
    username : "User1"
  }

  REST.post( 'userDetails', vars ).then( function( res ){
    //JSON.stringify nur für Ausgabe, alle infos stehen in res.details
    print( JSON.stringify( res.details ) );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//ADD OFFER
//********************************************************

function addOffer(){
  var randomNumber = Math.floor( Math.random() * 10 );
  var vars = {
    sessionID : SID,
    titel : "RandomOffer " + randomNumber,
    amount : randomNumber,
    duration : randomNumber
  }

  REST.post( 'addOffer', vars ).then( function( res ){
    print( "added Offer with " + SID );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//TAKE OFFER
//********************************************************

function takeRandomOffer(){
  var randomNumber = Math.floor( Math.random() * 3 );
  var AnotherRandomNumber = Math.floor( Math.random() * 10 );
  var vars = {
    sessionID : SID,
    id : randomNumber,
    amount : AnotherRandomNumber
  }

  REST.post( 'takeOffer', vars ).then( function( res ){
    print( "took " + AnotherRandomNumber + " of id: " + randomNumber );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//OFFERS AVALIABLE
//********************************************************

function offersAvaliable(){
  var vars = {
    sessionID : SID
  }

  REST.post( 'offersAvaliable', vars ).then( function( res ){
    //JSON.stringify nur für Ausgabe, alle infos stehen in res.offers
    print( JSON.stringify( res.offers ) );
  }).catch(function( e ){
    print( e );
  });
}

//********************************************************
//OFFERS OF USER
//********************************************************

function offersOfLoggedIn(){
  var vars = {
    sessionID : SID,
    username : myUsername
  }

  REST.post( 'offersUser', vars ).then( function( res ){
    //JSON.stringify nur für Ausgabe, alle infos stehen in res.offers
    print( JSON.stringify( res.offers ) );
  }).catch(function( e ){
    print( e );
  });
}
