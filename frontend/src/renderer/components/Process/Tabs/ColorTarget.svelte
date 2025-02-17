<script>
  import {
    customRefData,
    processState,
    modal,
    sendMessage,
  } from "@util/stores";
  import ColorTargetViewer from "@components/Process/ColorTargetViewer.svelte";

  import {
    PlusCircleIcon,
    XCircleIcon,
    AlertTriangleIcon,
  } from "svelte-feather-icons";
  import Dropdown from "@root/components/Dropdown.svelte";

  let colorTarget;
  let colorPos;
  let verifyTarget;
  let verifyPos;
  let viewerOpen;

  let colorTargetViewer;

  let targetArray;

  let loading = false;

  let refData = [
    "NGT_Reflectance_Data.csv",
    "APT_Reflectance_Data.csv",
    "CCSG_Reflectance_Data.csv",
    "CC_Classic_Reflectance_Data.csv",
    "Choose a custom file....csv",
  ];

  let refDataMeta = [
    { rows: 10, cols: 13 },
    { rows: 4, cols: 6 },
    { rows: 10, cols: 14 },
    { rows: 4, cols: 6 },
  ];

  function update() {
    if (colorPos) {
      $processState.artStacks[0].colorTarget.top = colorPos.top;
      $processState.artStacks[0].colorTarget.left = colorPos.left;
      $processState.artStacks[0].colorTarget.bottom = colorPos.bottom;
      $processState.artStacks[0].colorTarget.right = colorPos.right;
      $processState.artStacks[0].colorTarget.rows = colorTarget.rows;
      $processState.artStacks[0].colorTarget.cols = colorTarget.cols;
      $processState.artStacks[0].colorTarget.size = colorTarget.size;
      $processState.artStacks[0].colorTarget.resolution =
        colorTargetViewer.getResolution();
      $processState.artStacks[0].colorTarget.whitePatch =
        colorTarget.whitePatch;
      $processState.artStacks[0].colorTarget.refData = colorTarget.refData;
      if (colorTarget.refData.name !== "CUSTOM DATA") {
        $processState.artStacks[0].colorTarget.refData.name =
          colorTarget.refData.name;
      } else {
        $processState.artStacks[0].colorTarget.refData.name =
          $customRefData.calibration.name;
      }
    }

    if (verifyPos) {
      $processState.artStacks[0].verificationTarget.top = verifyPos.top;
      $processState.artStacks[0].verificationTarget.left = verifyPos.left;
      $processState.artStacks[0].verificationTarget.bottom = verifyPos.bottom;
      $processState.artStacks[0].verificationTarget.right = verifyPos.right;
      $processState.artStacks[0].verificationTarget.rows = verifyTarget.rows;
      $processState.artStacks[0].verificationTarget.cols = verifyTarget.cols;
      $processState.artStacks[0].verificationTarget.size = verifyTarget.size;
      $processState.artStacks[0].verificationTarget.resolution =
        colorTargetViewer.getResolution();
      $processState.artStacks[0].verificationTarget.whitePatch =
        verifyTarget.whitePatch;
      $processState.artStacks[0].verificationTarget.refData =
        verifyTarget.refData;
      if (verifyTarget.refData.name !== "CUSTOM DATA") {
        $processState.artStacks[0].verificationTarget.refData.name =
          verifyTarget.refData.name;
      }
    } else {
      $processState.artStacks[0].verificationTarget = null;
    }

    colorTarget = null;
    colorPos = null;
    verifyTarget = null;
    verifyPos = null;

    targetArray = null;

    loading = false;
  }

  function colorTargetPrev() {
    $processState.colorTargetID = Math.floor(Math.random() * 999999999);
    console.log($processState);

    let targetImage = "";
    


    if ($processState.artStacks[0].fields.targetA.length !== 0) {
      console.log("Found Target");
      targetImage = $processState.artStacks[0].fields.targetA[0].name;
    }
    else{
      console.log("Didnt Found Target");
      targetImage = $processState.artStacks[0].fields.imageA[0][0].name; 
    }

    let msg = {
      RequestID: $processState.colorTargetID,
      RequestType: "HalfSizePreview",
      RequestData: {
        names: [targetImage],
      },
    };
    if (
     // $processState.artStacks[0].fields.imageA[0].name.length > 2 &&
      !loading
    ) {
      console.log("Getting Color Target Preview");
      console.log(msg);
      loading = true;
      sendMessage(JSON.stringify(msg));
    }
  }

  $: if ($processState.currentTab === 6) {
    console.log("Update");
    console.log($processState);
    update();
    console.log($processState);
  }

  $: if (
    $processState.currentTab === 5 &&
    $processState.artStacks[0].colorTargetImage?.filename?.length === 0 &&
    $processState.artStacks[0].fields.imageA[0] != null
  ) {
    console.log("Getting Color Target Preview");
    console.log($processState);
    colorTargetPrev();
  }

  $: console.log($processState);

  function addTarget() {
    if (!colorTarget) {
      colorTarget = {
        name: "Calibration Target",
        rows: 10,
        cols: 10,
        color: Math.floor(Math.random() * (360 - 0 + 1) + 0),
        size: 0.5,
        whitePatch: {
          row: null,
          col: null,
        },
        refData: {
          name: "---None---.csv",
          standardObserver: 1931,
          illuminants: "D50",
        },
      };
      colorPos = { top: 0.25, left: 0.25, bottom: 0.5, right: 0.5 };
    } else if (!verifyTarget) {
      verifyTarget = {
        name: "Verification Target",
        rows: 10,
        cols: 10,
        color: Math.floor(Math.random() * (360 - 0 + 1) + 0),
        size: 0.5,
        whitePatch: {
          row: 1,
          col: 1,
        },
        refData: {
          name: "---None---.csv",
          standardObserver: 1931,
          illuminants: "D50",
        },
      };
      verifyPos = { top: 0.25, left: 0.25, bottom: 0.5, right: 0.5 };
    }
  }

  function removeTarget(id) {
    console.log("Remove");
    if (id === 0) {
      if (verifyTarget == null) {
        console.log("Removing Color Target");
        colorTarget = null;
        colorPos = null;
      }
    } else if (id === 1) {
      console.log("Removing Verify Target");
      verifyTarget = null;
      verifyPos = null;
    }
  }

  $: if (colorTarget) {
    let root = document.documentElement;
    root.style.setProperty("--color_hue", `${colorTarget.color}`);
  }

  $: if (verifyTarget) {
    let root = document.documentElement;
    root.style.setProperty("--verfiy_hue", `${verifyTarget}`);
  }

  $: {
    targetArray = [];
    if (colorTarget) {
      targetArray = [colorTarget];
    }
    if (verifyTarget) {
      targetArray = [...targetArray, verifyTarget];
    }
  }

  // If processing is terminated by a server error or something else, then re-enter the previous data
  $: if ($processState.returnedFromProcessing) {
    resetTargets();
  }

  function resetTargets() {
    if ($processState.artStacks[0].colorTarget && Object.keys($processState.artStacks[0].colorTarget).length !== 0) {
      addDataToTarget()
    }
    if ($processState.artStacks[0].verificationTarget && Object.keys($processState.artStacks[0].verificationTarget).length !== 0) {
  addDataToTarget();
  }
  $processState.returnedFromProcessing = false;
  }

  // Add data to targets from $processState
  function addDataToTarget() {
  if (!colorTarget) {
  colorTarget = {
  name: "Calibration Target",
  rows: $processState.artStacks[0].colorTarget.rows,
  cols: $processState.artStacks[0].colorTarget.cols,
  color: Math.floor(Math.random() * (360 - 0 + 1) + 0),
  size: $processState.artStacks[0].colorTarget.size,
  whitePatch: {
  row: $processState.artStacks[0].colorTarget.whitePatch.row,
  col: $processState.artStacks[0].colorTarget.whitePatch.col,
  },
  refData: {
  name: $processState.artStacks[0].colorTarget.refData.name,
  standardObserver: $processState.artStacks[0].colorTarget.refData.standardObserver,
  illuminants: $processState.artStacks[0].colorTarget.refData.illuminants,
  },
  };
  colorPos = { top: $processState.artStacks[0].colorTarget.top, left: $processState.artStacks[0].colorTarget.left,
  bottom: $processState.artStacks[0].colorTarget.bottom, right: $processState.artStacks[0].colorTarget.right };
  } else if (!verifyTarget) {
  verifyTarget = {
  name: "Verification Target",
  rows: $processState.artStacks[0].verificationTarget.rows,
  cols: $processState.artStacks[0].verificationTarget.cols,
  color: Math.floor(Math.random() * (360 - 0 + 1) + 0),
  size: $processState.artStacks[0].verificationTarget.size,
  whitePatch: {
  row: $processState.artStacks[0].verificationTarget.whitePatch.row,
  col: $processState.artStacks[0].verificationTarget.whitePatch.col,
  },
  refData: {
  name: $processState.artStacks[0].verificationTarget.refData.name,
  standardObserver: $processState.artStacks[0].verificationTarget.refData.standardObserver,
  illuminants: $processState.artStacks[0].verificationTarget.refData.illuminants,
  },
  };
  verifyPos = { top: $processState.artStacks[0].verificationTarget.top, left: $processState.artStacks[0].verificationTarget.left,
  bottom: $processState.artStacks[0].verificationTarget.bottom, right: $processState.artStacks[0].verificationTarget.right };
  }
  }

  $: if (colorTarget?.refData?.name === "Choose a custom file....csv") {
  console.log("OPENING CUSTOM REF MODAL");
  modal.set("CustomRefData");
  let customFileName = colorTarget.refData.name.endsWith('.csv') ? colorTarget.refData.name : `${colorTarget.refData.name}.csv`;
  refData.push(customFileName);
  colorTarget.refData.name = "customFileName";
  }
  $: if (verifyTarget?.refData?.name === "Choose a custom file....csv") {
  console.log("OPENING CUSTOM REF MODAL VER");
  modal.set("CustomRefDataVer");
  let customFileName = verifyTarget.refData.name.endsWith('.csv') ? verifyTarget.refData.name : `${verifyTarget.refData.name}.csv`;
  refData.push(customFileName);
  verifyTarget.refData.name = "CUSTOM DATA";
  }

  $: if (refData.includes(colorTarget?.refData?.name)) {
  console.log("Setting Refdata ROW/COl");
  let index = refData.findIndex((x) => x === colorTarget.refData.name);

  colorTarget.rows = refDataMeta[index].rows;
  colorTarget.cols = refDataMeta[index].cols;
  }
  $: if (refData.includes(verifyTarget?.refData?.name)) {
  let index = refData.findIndex((x) => x === verifyTarget.refData.name);

  verifyTarget.rows = refDataMeta[index].rows;
  verifyTarget.cols = refDataMeta[index].cols;
  }

  $: console.log({ LOADING: loading });

  $: if (
  colorTarget &&
    colorTarget.refData.name !== "---None---.csv" &&
    (!verifyTarget || verifyTarget.refData?.name !== "---None---.csv") &&
    colorTarget?.whitePatch?.row &&
    colorTarget?.whitePatch?.col
  ) {
    $processState.completedTabs[5] = true;
  } else {
     $processState.completedTabs[5] = false;
  }

  $: if (colorTarget) {
    $processState.whitePatchFilled =
      colorTarget.whitePatch?.row && colorTarget.whitePatch?.col;
  }
</script>

<main>
  <div class="left">
    <div class="title">Target Patch Selection</div>
    <div class="left-content">
      <div class="image-container">
        {#if loading}
          <div class="loading">
            <div class="loading-box">Loading<span class="loader" /></div>
          </div>
        {/if}
        <ColorTargetViewer
          bind:colorTarget
          bind:verifyTarget
          bind:colorPos
          bind:verifyPos
          bind:loading
          bind:viewerOpen
          bind:this={colorTargetViewer}
        />
      </div>
    </div>
  </div>
  <div class="right">
    <!-- <div class="boxHead">Targets</div> -->
    <div class="cardBox">
      {#if viewerOpen}
      {#each [...targetArray, "Add"] as target, i (target)}
        {#if target === "Add" && i < 2}
          <div
            class="addCard"
            on:click={() => addTarget()}
            class:verificationAdd={i === 1}
          >
            <div class="clickHere">Click Here</div>
            {#if i === 0}
              Add a patch selection grid for calibration
            {:else}
              Add a patch selection grid for verification
            {/if}
            <PlusCircleIcon size="2x" />
          </div>
        {:else if target !== "Add"}
          <div
            class="card"
            class:colorTarget={i === 0}
            class:verificationTarget={i !== 0}
          >
            <h2>{target.name}</h2>
            <input
              type="range"
              class="colorSlider"
              bind:value={target.color}
              max="360"
            />

            <div class="rowcol">
              <div class="inputGroup">
                <span>Rows:</span>
                <input
                  placeholder="1..26 [a-z]"
                  type="number"
                  min="1"
                  bind:value={target.rows}
                />
              </div>
              <div class="inputGroup">
                <span>Columns:</span>
                <input
                  placeholder="1..26 [a-z]"
                  type="number"
                  min="1"
                  bind:value={target.cols}
                />
              </div>
            </div>
            <div class="refDataDiv">
              <span class="validatedTitle">
                {#if target.refData.name === "---None---.csv"}
                  <span class="invalid"
                    ><AlertTriangleIcon size="1.5x" />
                  </span>
                {/if}
                Reference Data:</span
              >
              <Dropdown
                values={refData}
                bind:selected={target.refData.name}
                invalid={target.refData.name === "---None---.csv"}
                spaceLast
              />
            </div>
            <div class="sizeDiv">
              <span>Selection Area Size:</span>
              {Math.round(target.size * 100)}%
              <input
                type="range"
                bind:value={target.size}
                min=".3"
                max=".7"
                step=".01"
              />
            </div>
            {#if i !== 1}
              <div class="break" />
              <div class="whitePatchBox">
                {#if target.whitePatch.row === null || target.whitePatch.col === null}
                  <span class="invalid"
                    ><AlertTriangleIcon size="1.5x" />
                  </span>
                {/if}
                <div class="whitePatchInfo">
                  <span> White Patch Location</span>
                  <div class="whitePatch">
                    <div class="inputGroup">
                      <span>Row: </span>
                      <input
                        placeholder="Row #"
                        type="number"
                        min="1"
                        max={target.rows}
                        bind:value={target.whitePatch.row}
                      />
                    </div>
                    <span class="and">&</span>
                    <div class="inputGroup">
                      <span>Col: </span>
                      <input
                        placeholder="Col #"
                        type="number"
                        min="1"
                        max={target.cols}
                        bind:value={target.whitePatch.col}
                      />
                    </div>
                  </div>
                </div>
              </div>
              <div class="target-coordinates">
                <h3>Color Target Coordinates</h3>
                <div class="inputGroup">
                  <span>Top:</span>
                  <input
                    type="number"
                    step="1"
                    value={colorPos.top * colorTargetViewer.getResolution()}
                    on:change={() => {
                      colorPos.top =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Bottom:</span>
                  <input
                    type="number"
                    step="1"
                    value={colorPos.bottom * colorTargetViewer.getResolution()}
                    on:change={() => {
                      colorPos.bottom =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Right:</span>
                  <input
                    type="number"
                    step="1"
                    value={colorPos.right * colorTargetViewer.getResolution()}
                    on:change={() => {
                      colorPos.right =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Left:</span>
                  <input
                    type="number"
                    step="1"
                    value={colorPos.left * colorTargetViewer.getResolution()}
                    on:change={() => {
                      colorPos.left =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateCoords();
                    }}
                  />
                </div>
              </div>
            {:else if target !== "Add"}
              <div class="target-coordinates">
                <h3>Color Target Coordinates</h3>
                <div class="inputGroup">
                  <span>Top:</span>
                  <input
                    type="number"
                    step="1"
                    value={verifyPos.top * colorTargetViewer.getResolution()}
                    on:change={() => {
                      verifyPos.top =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateVerifyCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Bottom:</span>
                  <input
                    type="number"
                    step="1"
                    value={verifyPos.bottom * colorTargetViewer.getResolution()}
                    on:change={() => {
                      verifyPos.bottom =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateVerifyCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Right:</span>
                  <input
                    type="number"
                    step="1"
                    value={verifyPos.right * colorTargetViewer.getResolution()}
                    on:change={() => {
                      verifyPos.right =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateVerifyCoords();
                    }}
                  />
                </div>
                <div class="inputGroup">
                  <span>Left:</span>
                  <input
                    type="number"
                    step="1"
                    value={verifyPos.left * colorTargetViewer.getResolution()}
                    on:change={() => {
                      verifyPos.left =
                        event.target.value / colorTargetViewer.getResolution();
                      colorTargetViewer.updateVerifyCoords();
                    }}
                  />
                </div>
              </div>
            {/if}
            <button
              class="close"
              disabled={i === 0 &&
                typeof verifyTarget != "undefined" &&
                verifyTarget != null}
              on:click={() => removeTarget(i)}
              ><XCircleIcon size="1.25x" /></button
            >
          </div>
        {/if}
      {/each}
      {/if}
    </div>
  </div>
</main>

<style lang="postcss">
  :root {
    --color_hue: 50;
    --verfiy_hue: 100;
  }

  main {
    @apply flex w-full h-full overflow-hidden;
  }

  .left {
    @apply w-full h-full flex flex-col justify-start bg-gray-600 p-2 m-1 gap-2;
  }

  .title {
    @apply text-3xl p-1;
  }

  .right {
    @apply w-[40vw] h-full flex flex-col m-1 bg-gray-700 pt-[2vh] pb-[8vh] items-center;
  }

  .cardBox {
    @apply bg-gray-800 min-h-[60vh] w-[85%] p-2 gap-2 flex flex-col items-center
	rounded-2xl overflow-y-auto overflow-x-hidden;
  }

  .card {
    @apply rounded-lg w-full h-auto p-[1.5vw] flex flex-col gap-2 relative font-semibold;
  }
  .invalid {
    @apply text-red-600;
  }
  .whitePatchBox {
    @apply w-full flex justify-center items-center gap-2;
  }
  .whitePatchInfo {
    @apply flex flex-col w-full;
  }

  .addCard {
    @apply w-full h-full max-h-[50%] bg-green-400/50 rounded-lg p-4 flex flex-col gap-2 relative
	justify-center items-center hover:bg-green-400/60 active:scale-95 transition-all
	active:bg-green-400/75;
  }
  .verificationAdd {
    @apply bg-gray-500/50 hover:bg-green-400/40 active:bg-green-400/50;
  }
  .clickHere {
    @apply text-2xl;
  }
  .left-content {
    @apply w-full h-auto flex items-center;
  }
  .image-container {
    @apply relative w-full h-auto bg-gray-800 overflow-visible;
  }
  .colorTarget {
    background-color: hsl(var(--color_hue), 100%, 30%);
  }
  .validatedTitle {
    @apply flex items-center justify-center gap-2;
  }

  .verificationTarget {
    background-color: hsl(var(--verfiy_hue), 100%, 30%);
  }

  h2 {
    @apply text-lg justify-center flex items-center font-semibold;
  }

  .rowcol {
    @apply flex flex-col justify-between items-center;
  }

  .whitePatch {
    @apply flex justify-between items-center gap-1 text-base;
  }

  .addTarget {
    @apply bg-green-500 w-16 h-16 transition-all rounded-full flex items-center justify-center
	text-gray-100;
  }

  .removeButton {
    @apply hidden;
  }

  .rowcol input {
    text-align: center;
    @apply p-0.5 bg-gray-900 border-2 border-gray-800 rounded-lg
	focus-visible:outline-blue-700 focus-visible:outline focus-visible:outline-2
	h-full w-12 min-w-[1rem];
  }

  .whitePatch input {
    @apply p-0.5 bg-gray-900 border-2 border-gray-800 rounded-lg
	focus-visible:outline-blue-700 focus-visible:outline focus-visible:outline-2
	h-full w-full;
  }

  .extra {
    @apply bg-gray-700 p-2;
  }

  .rowcol .inputGroup {
    @apply flex justify-between items-center gap-2 w-full;
  }

  .whitePatch .inputGroup {
    @apply flex items-center gap-2 w-full;
  }

  .inputGroup > span {
    @apply font-semibold;
  }
  .times {
    @apply text-xl;
  }

  .close {
    @apply absolute top-0 right-0 bg-transparent text-gray-100
	hover:bg-red-600/50 hover:text-white ring-0 p-1;
  }
  .refDataDiv {
    @apply flex justify-between items-center;
  }
  .break {
    @apply w-full h-1 bg-black border-2;
  }
  .colorSlider {
    -webkit-appearance: none;
    appearance: none;
    background: linear-gradient(
      to right,
      hsl(0, 100%, 40%),
      hsl(20, 100%, 40%),
      hsl(40, 100%, 40%),
      hsl(60, 100%, 40%),
      hsl(80, 100%, 40%),
      hsl(100, 100%, 40%),
      hsl(120, 100%, 40%),
      hsl(140, 100%, 40%),
      hsl(160, 100%, 40%),
      hsl(180, 100%, 40%),
      hsl(200, 100%, 40%),
      hsl(220, 100%, 40%),
      hsl(240, 100%, 40%),
      hsl(260, 100%, 40%),
      hsl(280, 100%, 40%),
      hsl(300, 100%, 40%),
      hsl(320, 100%, 40%),
      hsl(340, 100%, 40%),
      hsl(360, 100%, 40%)
    );
    @apply w-full h-2 rounded-xl;
  }
  .colorSlider::-webkit-slider-thumb {
    -webkit-appearance: none; /* Override default look */
    appearance: none;
    @apply w-4 h-4 bg-gray-600 cursor-pointer rounded-full outline outline-1
	outline-gray-200;
  }
  .sizeDiv {
    @apply flex justify-between items-center;
  }
  .sizeDiv input {
    @apply w-1/2;
  }

  .loading {
    @apply bg-gray-700 absolute w-full h-full z-[49] flex justify-center items-center;
  }
  .loading-box {
    @apply h-full flex flex-col gap-2 justify-center items-center
	text-2xl;
  }
  .loader {
    /* position: absolute; */
    width: 48px;
    height: 48px;
    background: #11ff00;
    transform: rotateX(65deg) rotate(45deg);
    /* remove bellows command for perspective change */
    transform: perspective(200px) rotateX(65deg) rotate(45deg);
    color: rgb(255, 0, 0);
    animation: layers1 1s linear infinite alternate;
    @apply z-50;
  }
  .loader:after {
    content: "";
    position: absolute;
    inset: 0;
    background: rgb(0, 0, 255);
    animation: layerTr 1s linear infinite alternate;
  }

  @keyframes layers1 {
    0% {
      box-shadow: 0px 0px 0 0px;
    }
    90%,
    100% {
      box-shadow: 20px 20px 0 -4px;
    }
  }
  @keyframes layerTr {
    0% {
      transform: translate(0, 0) scale(1);
    }
    100% {
      transform: translate(-25px, -25px) scale(1);
    }
  }

  .rowcol input {
    text-align: center;
    @apply p-0.5 bg-gray-900 border-2 border-gray-800 rounded-lg
	focus-visible:outline-blue-700 focus-visible:outline focus-visible:outline-2
	h-full w-12 min-w-[1rem];
  }

  .target-coordinates input {
    @apply p-0.5 bg-gray-900 border-2 border-gray-800 rounded-lg
	focus-visible:outline-blue-700 focus-visible:outline focus-visible:outline-2
	h-full w-full;
  }
</style>
