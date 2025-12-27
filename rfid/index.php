<?php
include "koneksi.php";
$data = mysqli_query($conn, "SELECT * FROM attendance ORDER BY scan_time DESC");
?>

<!DOCTYPE html>
<html>
<head>
<title>Sistem Absensi RFID</title>
<style>
body {
  background:#020617;
  color:white;
  font-family:Arial;
}
table {
  width:100%;
  border-collapse:collapse;
}
th, td {
  padding:10px;
  border-bottom:1px solid #334155;
}
</style>
</head>
<body>

<h2>📊 Sistem Absensi RFID</h2>

<table>
<tr>
  <th>UID</th>
  <th>Waktu</th>
</tr>

<?php while($row = mysqli_fetch_assoc($data)): ?>
<tr>
  <td><?= $row['uid'] ?></td>
  <td><?= $row['scan_time'] ?></td>
</tr>
<?php endwhile; ?>

</table>

</body>
</html>
