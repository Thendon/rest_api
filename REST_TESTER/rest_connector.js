//******************************************************************
//Restconnector
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

//let restURL = 'http://localhost:1337/'; //Lokale Datanbank einrichten falls online nicht klappt
let restURL = 'http://5.9.67.102:1337/'; //Sollte schon online klappen
let REST = {}

var postHandle = function(path, vars){
  return new Promise((resolve, reject) => {
    let http = new XMLHttpRequest();
    let url = restURL + path;
    http.open("POST", url, true);
    //Send the proper header information along with the request
    http.setRequestHeader("Content-type", "application/json");
    http.onload = function(){
      if(http.readyState == 4 && http.status == 200) {
        resolve(http);
      }
      reject(http.statusText);
    }
    http.onerror = function(){
      reject(http.responseText);
    }
    http.send( JSON.stringify(vars) );
  });
}

REST.post = function(path, vars){
  return new Promise((resolve, reject) => {
    postHandle( path, vars ).then( function(http){
      let res = JSON.parse( http.responseText );
      if( !res.success ){
        reject( "critical Server-Side-Error!" );
      }
      if( res.error ){
        reject( res.error );
      }
      resolve( res );
    }).catch( function(e){
      reject( "could not connect to server ( err: " + e + " )" );
    });
  });
}
