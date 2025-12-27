<?php
include "koneksi.php";

if (isset($_GET['uid'])) {
  $uid = $_GET['uid'];

  mysqli_query($conn, "INSERT INTO attendance (uid) VALUES ('$uid')");
  echo "OK";
} else {
  echo "NO UID";
}
?>
