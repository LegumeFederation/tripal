<?php
$featureloc_sequences = $variables['tripal_feature']['featureloc_sequences'];
if(count($featureloc_sequences) > 0){
   foreach($featureloc_sequences as $seq){ ?>
     <div id="tripal_feature-floc_seq-<?php print $attrs['type']?>-box" class="tripal_feature-info-box tripal-info-box">
       <div class="tripal_feature-info-box-title tripal-info-box-title">Formatted <?php print $attrs['type']?> sequence </div>
       <div class="tripal_feature-info-box-desc tripal-info-box-desc">The formatted <?php print $attrs['type']?> sequence </div>
       <?php print $attrs['formatted_seq'] ?>
     <div>        
   <?php } 
}?>

